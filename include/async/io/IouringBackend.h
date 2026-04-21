#ifndef _ASYNC_IO_URING_BACKEND_H_
#define _ASYNC_IO_URING_BACKEND_H_
#ifndef _WIN32

#include "IoBackend.h"

#include <liburing.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <thread>
#include <atomic>
#include <mutex>
#include <cassert>

// ============================================================
// IouringOp — конкретна операція для io_uring
//
// Зберігає sqe_prep — функцію що заповнює SQE.
// submit() отримує SQE з рингу і заповнює його.
// user_data = this — completion thread знаходить нас по CQE.
// ============================================================
class IouringBackend;

struct IouringOp final : IoOp {
    enum class Type { Read, Write, Fsync } type = Type::Read;
    file_handle_t fd = static_cast<file_handle_t>(-1);
    void* buf = nullptr;
    size_t   len = 0;
    int64_t  offset = 0;

protected:
    // Делегує submit своєму io_uring backend-у.
    void submit() noexcept override;
    // submit_if_sync() — не перевизначаємо (no-op з базового класу)
    // io_uring завжди async — await_ready() завжди false
};

// ============================================================
// IouringBackend
//
// Один io_uring рінг на весь процес.
// Completion thread крутить цикл io_uring_wait_cqe() і
// відновлює корутини через IoOp::complete().
//
// Потокобезпека:
//   - prep_* і submit() захищені mutex (SQ не thread-safe)
//   - CQ читається тільки completion thread-ом
// ============================================================
class IouringBackend final : public IoBackend {
public:
    static constexpr unsigned QUEUE_DEPTH = 256;

    // Ініціалізує io_uring і фіксує, чи backend доступний на цій системі.
    explicit IouringBackend(unsigned queue_depth = QUEUE_DEPTH) {
        int ret = io_uring_queue_init(queue_depth, &ring_, 0);
        available_ = (ret == 0);
    }

    // Завершує completion loop і звільняє ring-ресурси.
    ~IouringBackend() override {
        stop();
        if (available_) io_uring_queue_exit(&ring_);
    }

    // Повертає, чи вдалося ініціалізувати io_uring.
    bool is_available() const noexcept { return available_; }

    // ── lifecycle ──────────────────────────────────────────
    // Запускає completion loop у фоновому потоці.
    void start() override {
        assert(available_);
        running_.store(true, std::memory_order_release);
        thread_ = std::thread([this] { completion_loop(); });
    }

    // Зупиняє completion loop і коректно завершує worker-потік.
    void stop() override {
        if (!running_.exchange(false, std::memory_order_acq_rel))
            return;

        // nop — будимо completion loop
        {
            std::lock_guard lock(sq_mtx_);
            struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
            if (sqe) {
                io_uring_prep_nop(sqe);
                io_uring_sqe_set_data(sqe, nullptr);
                io_uring_submit(&ring_);
            }
        }

        if (!thread_.joinable()) return;

        if (thread_.get_id() == std::this_thread::get_id()) {
            thread_.detach();
            return;
        }

        thread_.join();
    }

    // ── open / close ───────────────────────────────────────
    // Відкриває файл через POSIX open.
    file_handle_t open(const char* path, int flags, int mode) override {
        return static_cast<file_handle_t>(::open(path, flags, mode));
    }

    // Закриває POSIX file descriptor.
    void close(file_handle_t fd) override {
        ::close(static_cast<int>(fd));
    }

    // ── prep ───────────────────────────────────────────────
    // Готує read-операцію для наступного SQE.
    void prep_read(IoOp* op, file_handle_t fd,
        void* buf, size_t len, int64_t offset) noexcept override
    {
        auto* u = static_cast<IouringOp*>(op);
        u->backend = this;
        u->type = IouringOp::Type::Read;
        u->fd = fd;
        u->buf = buf;
        u->len = len;
        u->offset = offset;
    }

    // Готує write-операцію для наступного SQE.
    void prep_write(IoOp* op, file_handle_t fd,
        const void* buf, size_t len, int64_t offset) noexcept override
    {
        auto* u = static_cast<IouringOp*>(op);
        u->backend = this;
        u->type = IouringOp::Type::Write;
        u->fd = fd;
        u->buf = const_cast<void*>(buf);
        u->len = len;
        u->offset = offset;
    }

    // Готує fsync-операцію для наступного SQE.
    void prep_fsync(IoOp* op, file_handle_t fd) noexcept override {
        auto* u = static_cast<IouringOp*>(op);
        u->backend = this;
        u->type = IouringOp::Type::Fsync;
        u->fd = fd;
    }

    // ── submit SQE ─────────────────────────────────────────
    // Заповнює SQE і submit-ить його в ring.
    void submit_op(IouringOp* op) noexcept {
        std::lock_guard lock(sq_mtx_);

        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
        assert(sqe && "SQ full — збільш QUEUE_DEPTH");

        switch (op->type) {
        case IouringOp::Type::Read:
            io_uring_prep_read(sqe, op->fd,
                op->buf, static_cast<unsigned>(op->len),
                static_cast<uint64_t>(op->offset));
            break;
        case IouringOp::Type::Write:
            io_uring_prep_write(sqe, op->fd,
                op->buf, static_cast<unsigned>(op->len),
                static_cast<uint64_t>(op->offset));
            break;
        case IouringOp::Type::Fsync:
            io_uring_prep_fsync(sqe, op->fd, 0);
            break;
        }

        io_uring_sqe_set_data(sqe, op);
        io_uring_submit(&ring_);
    }

    // Адаптер для делегуючих IoOp-типів на кшталт UniOp.
    void submit_op(IoOp* base) noexcept override {
        submit_op(static_cast<IouringOp*>(base));
    }

    // ── синхронні ──────────────────────────────────────────
    // Виконує синхронний seek для відкритого descriptor-а.
    int64_t seek(file_handle_t fd, int64_t offset, int whence) noexcept override {
        return static_cast<int64_t>(
            ::lseek(static_cast<int>(fd), static_cast<off_t>(offset), whence));
    }

    // Обрізає файл до нового розміру.
    bool truncate(file_handle_t fd, int64_t size) noexcept override {
        return ::ftruncate(static_cast<int>(fd), static_cast<off_t>(size)) == 0;
    }

    // Повертає поточний розмір файлу.
    int64_t file_size(file_handle_t fd) noexcept override {
        struct stat st {};
        if (::fstat(static_cast<int>(fd), &st) != 0) return -1;
        return static_cast<int64_t>(st.st_size);
    }

private:
    // Очікує CQE, завершує await-операції і drain-ить залишки під час stop().
    void completion_loop() {
        while (running_.load(std::memory_order_acquire)) {
            struct io_uring_cqe* cqe = nullptr;
            int ret = io_uring_wait_cqe(&ring_, &cqe);
            if (ret < 0) continue;

            IouringOp* op = static_cast<IouringOp*>(
                io_uring_cqe_get_data(cqe));
            int32_t res = cqe->res;
            io_uring_cqe_seen(&ring_, cqe);

            if (op) op->complete(res);
        }

        // drain залишків після stop()
        struct io_uring_cqe* cqe = nullptr;
        while (io_uring_peek_cqe(&ring_, &cqe) == 0 && cqe) {
            IouringOp* op = static_cast<IouringOp*>(
                io_uring_cqe_get_data(cqe));
            int32_t res = cqe->res;
            io_uring_cqe_seen(&ring_, cqe);
            if (op) op->complete(res);
        }
    }

    struct io_uring   ring_ {};
    bool              available_ = false;
    std::mutex        sq_mtx_;
    std::thread       thread_;
    std::atomic<bool> running_{ false };
};

// Запускає backend-specific submit для поточної io_uring операції.
inline void IouringOp::submit() noexcept {
    static_cast<IouringBackend*>(backend)->submit_op(this);
}

#endif // ! _WIN32
#endif // _ASYNC_IO_URING_BACKEND_H_