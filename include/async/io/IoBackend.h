#ifndef _ASYNC_IO_BACKEND_H_
#define _ASYNC_IO_BACKEND_H_

#include <coroutine>
#include <atomic>
#include <cstdint>
#include <cstddef>
#include "../error.h"
#include "../expected.h"

// =========================================================
// Результат I/O операції - те що повертає await_resume()
// =========================================================

struct IoResult {
	int32_t bytes = 0;  // > 0 = байт прочитано/записано
						// = 0 = EOF або fsync ok
						// < 0 = -errno (Linux) / -GetLastError() (Win)
	// Перевіряє, чи завершилась I/O операція без помилки.
	bool ok() const noexcept { return bytes >= 0; }
};

// ============================================================
// IoBackend — forward declaration
// ============================================================
class IoBackend;
using file_handle_t = intptr_t;

// ============================================================
// IoOp — Awaitable для async I/O операцій.
//
// Підтримує два режими:
//
// Async (io_uring, IOCP):
//   await_ready() → false
//   await_suspend() → submit() реєструє в ядрі, підвішуємось
//   completion thread → complete() → handle.resume()
//
// Sync (SyncMockBackend, fsync):
//   submit() виконується і одразу викликає complete()
//   complete() бачить handle=nullptr → тільки ставить ready=true
//   await_ready() → true → await_suspend() НЕ викликається
//   await_resume() → повертає результат
//
// Ключовий інваріант: submit() викликається ПІСЛЯ await_suspend
// встановив handle — тоді complete() безпечно викликає resume().
// Для sync backend-ів треба викликати submit() в await_ready()
// коли handle ще nullptr → complete() тільки ставить ready=true
// → await_ready()=true → suspend не відбувається.
// ============================================================
struct IoOp {
	std::coroutine_handle<> handle	= nullptr;
	IoResult				result	= {};
	std::atomic<bool>		ready{ false };
	std::atomic<bool>		submitting{ false };
	IoBackend*				backend	= nullptr; // встановлюється в prep_*

	// Викликається completion thread-ом після завершення операції
	// Зберігає результат і, за потреби, відновлює continuation.
	void complete(int32_t res) noexcept {
		result.bytes = res;
		ready.store(true, std::memory_order_release);
		
		// якщо handle встановлений — async режим, resume корутину
		// якщо nullptr — sync режим, await_ready побачить ready=true
		if (handle && !submitting.load(std::memory_order_acquire)) {
			auto h = handle;
			handle = nullptr;
			h.resume();
		}
	}

	// ── Awaitable interface ────────────────────────────────

	// Для sync backend-ів: викликаємо submit() тут щоб complete()
	// спрацював до await_suspend (handle ще nullptr → тільки ready=true)
	// Вирішує, чи можна завершити await без реального suspend.
	bool await_ready() noexcept {
		if (!ready.load(std::memory_order_acquire) && backend != nullptr) {
			// Викликаємо submit() для sync перевірки.
			// Async backend-и (io_uring/IOCP) мають submit() що
			// потребує handle — вони ігнорують виклик тут і
			// виконують реальний submit в await_suspend.
			submit_if_sync();
		}
		return ready.load(std::memory_order_acquire);
	}

	// Повертає результат завершеної I/O операції.
	IoResult await_resume() const noexcept { return result; }

	// Запам'ятовує continuation і делегує submit конкретному op-типу.
	bool await_suspend(std::coroutine_handle<> h) noexcept {
		// await_ready повернув false → async режим
		// handle ще не встановлений → встановлюємо і submit-имо
		handle = h;
		submitting.store(true, std::memory_order_release);
		submit(); // реєструє в ядрі (io_uring SQE або IOCP ReadFile)
		submitting.store(false, std::memory_order_release);
		return !ready.load(std::memory_order_acquire);
	}

protected:
	// Реальний submit для async backend-ів або делегуючих op-типів.
	virtual void submit() noexcept = 0;
	
	// Для sync backend-ів — перевизначається щоб виконати операцію
	// синхронно в await_ready() (до встановлення handle)
	// Hook для sync backend-ів, які можуть завершити I/O до suspend.
	virtual void submit_if_sync() noexcept {
		// default: нічого — async backend-и не виконують тут
	}

	virtual ~IoOp() = default;
};

// ===========================================================
// IoBackend - абстрактний інтефейс для платформи.
// IouringBackend (Linux) i IocpBackend (Windows) реалізують його.
// ===========================================================
class IoBackend
{
public:
	virtual ~IoBackend() = default;

	// Запустити completion loop в окремому потоці
	virtual void start() = 0;

	// Зупинити completion loop в дочекатися завершення потоку
	virtual void stop() = 0;

	// Відкрити файл асинхроно - повертає fd/handle або -1
	virtual file_handle_t open(const char* path, int flags, int mode) = 0;

	// Закрити файл
	virtual void close(file_handle_t fd) = 0;

	// Створити IoOp для read
	// op живе на стеку корунтини - backend тримає вказівник тільки під час виконнання
	virtual void prep_read(IoOp* op, file_handle_t fd, void* buffer, size_t len, int64_t offset) noexcept = 0;
	virtual void prep_write(IoOp* op, file_handle_t fd, const void* buffer, size_t len, int64_t offset) noexcept = 0;
	virtual void prep_fsync(IoOp* op, file_handle_t fd) noexcept = 0;

	// Seek i truncate - синчроні на обох платформах ( немає async варіантів в io_uring для lseek)
	virtual int64_t seek(file_handle_t fd, int64_t offset, int whence) noexcept = 0;
	virtual bool truncate(file_handle_t fd, int64_t size) noexcept = 0;
	virtual int64_t file_size(file_handle_t fd) noexcept = 0;

	// Єдина точка входу для backend-специфічного submit.
	// UniOp делегує сюди, а конкретний backend вирішує
	// чи це async submit, чи sync execution.
	virtual void submit_op(IoOp* /*op*/) noexcept {}

	// true для sync backend-ів, які виконують I/O одразу.
	virtual bool is_sync() const noexcept { return false; }
};

#endif // _ASYNC_IO_BACKEND_H_