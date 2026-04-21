#ifndef _ASYNC_IO_IOCP_BACKEND_H_
#define _ASYNC_IO_IOCP_BACKEND_H_
#ifdef _WIN32

#include "IoBackend.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fcntl.h>

#include <thread>
#include <atomic>
#include <cassert>

// ============================================================
// IocpOp
//
// OVERLAPPED зберігається як member і completion_loop відновлює
// IocpOp через CONTAINING_RECORD(ov, IocpOp, ov).
// ============================================================
class IocpBackend;

struct IocpOp final : IoOp {
    OVERLAPPED ov = {};
    HANDLE     file = INVALID_HANDLE_VALUE;
    void* buf = nullptr;
    DWORD      len = 0;
    bool       is_read = false;

    // Записує 64-бітний файловий offset у поля OVERLAPPED.
    void set_offset(int64_t offset) noexcept {
        ov.Offset = static_cast<DWORD>(offset & 0xFFFFFFFF);
        ov.OffsetHigh = static_cast<DWORD>((offset >> 32) & 0xFFFFFFFF);
    }

protected:
    // Делегує submit своєму IOCP backend-у.
    void submit() noexcept override;   // реалізація після IocpBackend
    // submit_if_sync() — no-op (IOCP завжди async крім FlushFileBuffers)
};

// ============================================================
// IocpBackend
//
// Модель завершення IOCP відрізняється від io_uring:
//
//   io_uring: ми самі submit-имо SQE → ядро виконує → CQE в рингу
//   IOCP:     ми викликаємо ReadFile/WriteFile з &op.ov →
//             Windows сам постить пакет на порт після завершення →
//             GetQueuedCompletionStatus повертає OVERLAPPED*
//
// Важливо: файл ПОВИНЕН бути відкритий з FILE_FLAG_OVERLAPPED,
// інакше ReadFile блокується і IOCP не отримає пакет.
// ============================================================
class IocpBackend final : public IoBackend {
public:
    // Створює IOCP port і готує backend до старту completion loop.
    explicit IocpBackend(DWORD concurrent_threads = 1) {
        port_ = CreateIoCompletionPort(
            INVALID_HANDLE_VALUE, NULL, 0, concurrent_threads);
        assert(port_ != NULL && "CreateIoCompletionPort failed");
    }

    // Гарантує зупинку completion thread і закриття порту.
    ~IocpBackend() override {
        stop();
        if (port_) CloseHandle(port_);
    }

    // ── lifecycle ──────────────────────────────────────────
    // Запускає completion loop в окремому потоці.
    void start() override {
        running_.store(true, std::memory_order_release);
        thread_ = std::thread([this] { completion_loop(); });
    }

    // Зупиняє completion loop і коректно завершує worker-потік.
    void stop() override {
        if (!running_.exchange(false, std::memory_order_acq_rel))
            return;

        // wakeup: постимо пакет з overlapped = nullptr
        PostQueuedCompletionStatus(port_, 0, 0, nullptr);

        if (!thread_.joinable()) return;

        if (thread_.get_id() == std::this_thread::get_id()) {
            thread_.detach();
            return;
        }

        thread_.join();
    }

    // ── open / close ───────────────────────────────────────
    // flags — POSIX-сумісні (O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_TRUNC)
    // Відкриває файл у overlapped-режимі та прив'язує його до IOCP.
    file_handle_t open(const char* path, int flags, int /*mode*/) override {
        DWORD access = 0;
        DWORD creation = OPEN_EXISTING;
        DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE;
        // FILE_FLAG_OVERLAPPED — ОБОВ'ЯЗКОВО для IOCP
        DWORD attr = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;

        if (flags & O_RDONLY || flags & O_RDWR) access |= GENERIC_READ;
        if (flags & O_WRONLY || flags & O_RDWR) access |= GENERIC_WRITE;
        if (flags & O_APPEND)                   access |= FILE_APPEND_DATA;

        if (flags & O_CREAT && flags & O_TRUNC) creation = CREATE_ALWAYS;
        else if (flags & O_CREAT)                    creation = OPEN_ALWAYS;
        else if (flags & O_TRUNC)                    creation = TRUNCATE_EXISTING;

        HANDLE h = CreateFileA(
            path, access, share, nullptr, creation, attr, nullptr);

        if (h == INVALID_HANDLE_VALUE) return -1;

        // Асоціюємо handle з нашим IOCP портом.
        // Після цього кожне завершення ReadFile/WriteFile
        // автоматично постить CQE на port_.
        HANDLE res = CreateIoCompletionPort(
            h, port_, reinterpret_cast<ULONG_PTR>(h), 0);

        if (!res) { CloseHandle(h); return -1; }

        // Повертаємо HANDLE як int через intptr_t
        return reinterpret_cast<file_handle_t>(h);
    }

    // Закриває native Windows file handle.
    void close(file_handle_t fd) override {
        auto h = fd_to_handle(fd);
        if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
    }

    // ── prep ───────────────────────────────────────────────
    // Готує read-операцію для подальшого ReadFile(..., &ov).
    void prep_read(IoOp* base, file_handle_t fd,
        void* buf, size_t len, int64_t offset) noexcept override
    {
        auto* op = static_cast<IocpOp*>(base);
        op->backend = this;
        op->file = fd_to_handle(fd);
        op->buf = buf;
        op->len = static_cast<DWORD>(len);
        op->is_read = true;
        op->set_offset(offset);
        // обнуляємо hEvent — не використовуємо event-based сповіщення
        op->ov.hEvent = nullptr;
    }

    // Готує write-операцію для подальшого WriteFile(..., &ov).
    void prep_write(IoOp* base, file_handle_t fd,
        const void* buf, size_t len, int64_t offset) noexcept override
    {
        auto* op = static_cast<IocpOp*>(base);
        op->backend = this;
        op->file = fd_to_handle(fd);
        op->buf = const_cast<void*>(buf);
        op->len = static_cast<DWORD>(len);
        op->is_read = false;
        op->set_offset(offset);
        op->ov.hEvent = nullptr;
    }

    // Готує fsync-операцію через FlushFileBuffers.
    void prep_fsync(IoOp* base, file_handle_t fd) noexcept override {
        auto* op = static_cast<IocpOp*>(base);
        op->backend = this;
        op->file = fd_to_handle(fd);
        op->buf = nullptr;  // маркер fsync
        op->len = 0;
    }

    // submit_op — викликається з IocpOp::submit() через await_suspend
    // Реєструє I/O в ядрі або синхронно виконує fsync.
    void submit_op(IocpOp* op) noexcept {
        // fsync — FlushFileBuffers синхронний
        if (op->buf == nullptr && op->len == 0) {
            BOOL ok = FlushFileBuffers(op->file);
            op->complete(ok ? 0 : -static_cast<int32_t>(GetLastError()));
            return;
        }

        BOOL ok;
        if (op->is_read) {
            ok = ReadFile(op->file, op->buf, op->len, nullptr, &op->ov);
        }
        else {
            ok = WriteFile(op->file, op->buf, op->len, nullptr, &op->ov);
        }

        // ERROR_IO_PENDING — нормально, операція поставлена в чергу IOCP
        if (!ok && GetLastError() != ERROR_IO_PENDING) {
            op->complete(-static_cast<int32_t>(GetLastError()));
        }
        // при ERROR_IO_PENDING — completion_loop відновить корутину
    }

    // Адаптер для делегуючих IoOp-типів на кшталт UniOp.
    void submit_op(IoOp* base) noexcept override {
        submit_op(static_cast<IocpOp*>(base));
    }

    // ── синхронні ──────────────────────────────────────────
    // Виконує синхронний seek для відкритого file handle.
    int64_t seek(file_handle_t fd, int64_t offset, int whence) noexcept override {
        HANDLE h = fd_to_handle(fd);
        DWORD  method;
        switch (whence) {
        case SEEK_SET: method = FILE_BEGIN;   break;
        case SEEK_CUR: method = FILE_CURRENT; break;
        case SEEK_END: method = FILE_END;     break;
        default:       return -1;
        }
        LARGE_INTEGER li{}, result{};
        li.QuadPart = offset;
        if (!SetFilePointerEx(h, li, &result, method)) return -1;
        return result.QuadPart;
    }

    // Обрізає файл до нового розміру.
    bool truncate(file_handle_t fd, int64_t size) noexcept override {
        if (seek(fd, size, SEEK_SET) < 0) return false;
        return SetEndOfFile(fd_to_handle(fd)) != 0;
    }

    // Повертає поточний розмір файлу.
    int64_t file_size(file_handle_t fd) noexcept override {
        LARGE_INTEGER sz{};
        if (!GetFileSizeEx(fd_to_handle(fd), &sz)) return -1;
        return sz.QuadPart;
    }

private:
    // ── completion loop ────────────────────────────────────
    // Читає завершені пакети з IOCP і завершує відповідні await-операції.
    void completion_loop() {
        while (true) {
            DWORD      bytes = 0;
            ULONG_PTR  key = 0;
            OVERLAPPED* ov = nullptr;

            BOOL ok = GetQueuedCompletionStatus(
                port_, &bytes, &key, &ov, INFINITE);

            // nullptr overlapped = wakeup від stop()
            if (!ov) break;

            IocpOp* op = CONTAINING_RECORD(ov, IocpOp, ov);

            int32_t result = ok
                ? static_cast<int32_t>(bytes)
                : -static_cast<int32_t>(GetLastError());

            op->complete(result);
        }
    }

    // Перетворює абстрактний file_handle_t назад у Windows HANDLE.
    static HANDLE fd_to_handle(file_handle_t fd) noexcept {
        return reinterpret_cast<HANDLE>(fd);
    }

    HANDLE            port_ = NULL;
    std::thread       thread_;
    std::atomic<bool> running_{ false };
};

// Запускає backend-specific submit для поточної IOCP операції.
inline void IocpOp::submit() noexcept {
    static_cast<IocpBackend*>(backend)->submit_op(this);
}

#endif // _WIN32
#endif // _ASYNC_IO_IOCP_BACKEND_H_
