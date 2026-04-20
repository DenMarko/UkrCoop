#ifndef _ASYNC_IO_URING_BACKEND_H_
#define _ASYNC_IO_URING_BACKEND_H_

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

struct IouringOp final : IoOp
{
    enum class Type {Read, Write, Fsync} type = Type::Read;
    file_handle_t handle = static_cast<file_handle_t>(-1);
    void* buffer = nullptr;
    size_t size = 0;
    int64_t offset = 0;

protected:
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
class IouringBackend final : public IoBackend
{
public:
    static constexpr unsigned QUEUE_DEPTH = 256;

    explicit IouringBackend(unsigned queue_depth = QUEUE_DEPTH)
    {
        int ret = io_uring_queue_init(queue_depth, &ring_, 0);
        available_ = (ret == 0);
    }

    ~IouringBackend() override
    {
        stop();
        if (available_) io_uring_queue_exit(&ring_);
    }

    bool is_available() const noexcept { return available_; }

    // ── lifecycle ──────────────────────────────────────────
    void start() override
    {
        assert(available_);
        running_.store(true, std::memory_order_release);
        thread_ = std::thread([this] { completion_loop(); });
    }

    void stop() override
    {
        if(!running_.load(std::memory_order_acquire)) return; // вже зупинено

        // nop - будемо completion loop
        {
            std::lock_guard lock(sq_mtx_);
            struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
            if (sqe) {
                io_uring_prep_nop(sqe);
                io_uring_sqe_set_data(sqe, nullptr); // nullptr — сигнал для completion loop про зупинку
                io_uring_submit(&ring_);
            }
        }

        if(!thread_.joinable()) return; // thread не запущено

        if(thread_.get_id() == std::this_thread::get_id())
        {
            thread_.detach(); // якщо виклик з completion thread-а, то від'єднуємо його (завершиться після обробки nop)
            return;
        }

        thread_.join(); // чекаємо завершення completion thread-а
    }

    // ── open / close ───────────────────────────────────────
    file_handle_t open(const char* path, int flags, int mode) override
    {
        return static_cast<file_handle_t>(::open(path, flags, mode));
    }

    void close(file_handle_t handle) override
    {
        ::close(static_cast<int>(handle));
    }

    // ── prep_* ───────────────────────────────────────────
    void prep_read(IoOp *op, file_handle_t handle, void* buffer, size_t size, int64_t offset) noexcept override
    {
        IouringOp* iop = static_cast<IouringOp*>(op);
        iop->type = IouringOp::Type::Read;
        iop->handle = handle;
        iop->buffer = buffer;
        iop->size = size;
        iop->offset = offset;
    }

    void prep_write(IoOp *op, file_handle_t handle, const void* buffer, size_t size, int64_t offset) noexcept override
    {
        IouringOp* iop = static_cast<IouringOp*>(op);
        iop->type = IouringOp::Type::Write;
        iop->handle = handle;
        iop->buffer = const_cast<void*>(buffer); // io_uring не змінює буфер для запису
        iop->size = size;
        iop->offset = offset;
    }

    void prep_fsync(IoOp *op, file_handle_t handle) noexcept override
    {
        auto *u = static_cast<IouringOp*>(op);
        u->backend = this;
        u->type = IouringOp::Type::Fsync;
        u->handle = handle;
    }

    // ── submit SQE ─────────────────────────────────────────
    void submit_op(IouringOp *op) noexcept
    {
        std::lock_guard lock(sq_mtx_);
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
        assert(sqe && "SQE queue overflow — increase QUEUE_DEPTH or ensure timely completion");

        switch (op->type)
        {
        case IouringOp::Type::Read:
            io_uring_prep_read(sqe, 
                op->handle, op->buffer, 
                static_cast<unsigned>(op->size), 
                static_cast<uint64_t>(op->offset));
            break;
        case IouringOp::Type::Write:
            io_uring_prep_write(sqe, 
                op->handle, op->buffer, 
                static_cast<unsigned>(op->size), 
                static_cast<uint64_t>(op->offset));
            break;
        case IouringOp::Type::Fsync:
            io_uring_prep_fsync(sqe, op->handle, 0);
            break;
        }

        io_uring_sqe_set_data(sqe, op); // для completion loop-а
        io_uring_submit(&ring_);
    }

    // ── синхронні ──────────────────────────────────────────
    int64_t seek(file_handle_t handle, int64_t offset, int whence) noexcept override
    {
        return static_cast<int64_t>(::lseek(static_cast<int>(handle), static_cast<off_t>(offset), whence));
    }

    bool truncate(file_handle_t handle, int64_t size) noexcept override
    {
        return ::ftruncate(static_cast<int>(handle), static_cast<off_t>(size)) == 0;
    }

    int64_t file_size(file_handle_t handle) noexcept override
    {
        struct stat st{};
        if(::fstat(static_cast<int>(handle), &st) != 0)
        {
            return -1;
        }

        return static_cast<int64_t>(st.st_size);
    }

private:
    void completion_loop()
    {
        while(running_.load(std::memory_order_acquire))
        {
            struct io_uring_cqe* cqe = nullptr;
            int ret = io_uring_wait_cqe(&ring_, &cqe);
            if (ret < 0) continue; // помилка або сигнал — просто повторюємо цикл

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

    struct io_uring ring_ {};
    bool available_ = false;
    std::mutex sq_mtx_;
    std::thread thread_;
    std::atomic<bool> running_{ false };
};

inline void IouringOp::submit() noexcept
{
    static_cast<IouringBackend*>(backend)->submit_op(this);
}

#endif // _ASYNC_IO_URING_BACKEND_H_