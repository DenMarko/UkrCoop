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
struct IoResult
{
    int32_t bytes = 0;  // > 0 = байт прочитано/записано
						// = 0 = EOF або fsync ok
						// < 0 = -errno (Linux) / -GetLastError() (Win)
    bool ok() const noexcept { return bytes >= 0; }
};

// ============================================================
// IoBackend — forward declaration
// ============================================================
class IoBackend;
using file_handle_t = intptr_t; // Windows: HANDLE, Linux: int (fd)

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
struct IoOp
{
    std::coroutine_handle<>     handle = nullptr; // встановлюється в await_suspend
    IoResult                    result = {}; // встановлюється в complete()
    std::atomic<bool>           ready{ false }; // для sync backend-ів: submit() встановлює ready=true, complete() викликається до await_suspend()
    std::atomic<bool>           submitting{false}; // для sync backend-ів: щоб submit() викликався лише один раз
    IoBackend*                  backend = nullptr; // встановлюється в prep_*

    // Викликається completion thread-ом після завершення I/O операцій.
    void complete(int32_t res) noexcept
    {
        result.bytes = res;
        ready.store(true, std::memory_order_release);
        if (handle && !submitting.load(std::memory_order_acquire))
        {
            auto h = handle;
            handle = nullptr; // скидаємо handle, щоб не викликати resume() двічі
            h.resume();
        }
    }

    // -- Aweitable interface --
    // Для sync backend-ів: викликаємо submit() тут щоб complete()
	// спрацював до await_suspend (handle ще nullptr → тільки ready=true)
    bool await_ready() noexcept
    {
        if(!ready.load(std::memory_order_acquire) && backend != nullptr)
        {
            // Викликаємо submit() для sync перевірки.
			// Async backend-и (io_uring/IOCP) мають submit() що
			// потребує handle — вони ігнорують виклик тут і
			// виконують реальний submit в await_suspend.
            submit_if_sync();
        }
        return ready.load(std::memory_order_acquire);
    }

    IoResult await_resume() const noexcept { return result; }

    bool await_suspend(std::coroutine_handle<> h) noexcept
    {
    	// await_ready повернув false → async режим
		// handle ще не встановлений → встановлюємо і submit-имо
        handle = h;
        submitting.store(true, std::memory_order_release);
        submit(); // реєструє в ядрі (io_uring SQE або IOCP ReadFile)
        submitting.store(false, std::memory_order_release);
        return !ready.load(std::memory_order_acquire); // якщо ready стало true в submit(), то не підвішуємось
    }

protected:
    virtual void submit() noexcept = 0;
    virtual void submit_if_sync() noexcept
    {
        // За замовчуванням вважаємо, що це async backend, який ігнорує submit_if_sync()
        // Sync backend-и перевизначають цей метод, щоб викликати submit() при першому await_ready()
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
    virtual void close(file_handle_t handle) = 0;

    // Створити IoOp для read
	// op живе на стеку корунтини - backend тримає вказівник тільки під час виконнання
    virtual void prep_read(IoOp *op, file_handle_t handle, void* buffer, size_t size, int64_t offset) noexcept = 0;
    virtual void prep_write(IoOp *op, file_handle_t handle, const void* buffer, size_t size, int64_t offset) noexcept = 0;
    virtual void prep_fsync(IoOp *op, file_handle_t handle) noexcept = 0;

	// Seek i truncate - синчроні на обох платформах ( немає async варіантів в io_uring для lseek)
    virtual int64_t seek(file_handle_t handle, int64_t offset, int whence) noexcept = 0;
    virtual bool truncate(file_handle_t handle, int64_t size) noexcept = 0;
    virtual int64_t file_size(file_handle_t handle) noexcept = 0;
};

#endif // _ASYNC_IO_BACKEND_H_