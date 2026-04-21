// SyncMockBackend — для тестів і fallback на старих ядрах (< 5.1)
// Виконує I/O синхронно в submit_if_sync() → await_ready() = true
// → корутина ніколи не підвішується для I/O операцій

#ifndef _SYNC_MOCK_BACKEND_H_
#define _SYNC_MOCK_BACKEND_H_
#include "IoBackend.h"

#ifndef _WIN32
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <cerrno>
#else
#include <Windows.h>
#include <fcntl.h>
#endif

// SyncMockOp — простий IoOp що виконує I/O синхронно
// Параметри зберігаються прямо в op
struct SyncMockOp final : IoOp {
    enum class Type { Read, Write, Fsync } type = Type::Read;
    file_handle_t fd = static_cast<file_handle_t>(-1);
    void* buf = nullptr;
    size_t   len = 0;
    int64_t  offset = 0;

    // Виконує sync read/write/fsync і одразу завершує await.
    void do_io() noexcept {
        int32_t res = 0;
        switch (type) {
#ifndef _WIN32
        case Type::Read: {
            ssize_t n = ::pread(static_cast<int>(fd), buf, len, (off_t)offset);
            res = (n < 0) ? -errno : (int32_t)n;
            break;
        }
        case Type::Write: {
            ssize_t n = ::pwrite(static_cast<int>(fd), buf, len, (off_t)offset);
            res = (n < 0) ? -errno : (int32_t)n;
            break;
        }
        case Type::Fsync:
            res = (::fsync(static_cast<int>(fd)) == 0) ? 0 : -errno;
            break;
#else
        case Type::Read: {
            DWORD got = 0;
            auto h = reinterpret_cast<HANDLE>(fd);
            BOOL ok = ReadFile(h, buf, (DWORD)len, &got, nullptr);
            res = ok ? (int32_t)got : -(int32_t)GetLastError();
            break;
        }
        case Type::Write: {
            DWORD written = 0;
            auto h = reinterpret_cast<HANDLE>(fd);
            BOOL ok = WriteFile(h, buf, (DWORD)len, &written, nullptr);
            res = ok ? (int32_t)written : -(int32_t)GetLastError();
            break;
        }
        case Type::Fsync: {
            auto h = reinterpret_cast<HANDLE>(fd);
            res = FlushFileBuffers(h) ? 0 : -(int32_t)GetLastError();
            break;
        }
#endif
        }
        complete(res);
    }

protected:
    // У sync backend submit просто виконує I/O на місці.
    void submit()         noexcept override { do_io(); }
    // Дозволяє await_ready завершити I/O без suspend.
    void submit_if_sync() noexcept override { do_io(); }
};

class SyncMockBackend final : public IoBackend {
public:
    // Sync backend не має окремого completion thread.
    void start() noexcept override {}
    // Sync backend не потребує shutdown-логіки.
    void stop()  noexcept override {}

    // Відкриває файл через POSIX або WinAPI fallback.
    file_handle_t open(const char* path, int flags, int mode) override {
#ifndef _WIN32
        return static_cast<file_handle_t>(::open(path, flags, mode));
#else
        // спрощена Windows версія для тестів
        DWORD access = GENERIC_READ | GENERIC_WRITE;
        DWORD creation = (flags & O_CREAT) ?
            ((flags & O_TRUNC) ? CREATE_ALWAYS : OPEN_ALWAYS) : OPEN_EXISTING;
        HANDLE h = CreateFileA(path, access, FILE_SHARE_READ,
            nullptr, creation, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) return -1;
        return reinterpret_cast<file_handle_t>(h);
#endif
    }

    // Закриває native-дескриптор файлу.
    void close(file_handle_t fd) override {
#ifndef _WIN32
        ::close(static_cast<int>(fd));
#else
        CloseHandle(reinterpret_cast<HANDLE>(fd));
#endif
    }

    // Готує sync read-операцію в базовому IoOp.
    void prep_read(IoOp* op, file_handle_t fd,
        void* buf, size_t len, int64_t offset) noexcept override
    {
        auto* o = static_cast<SyncMockOp*>(op);
        o->backend = this;
        o->type = SyncMockOp::Type::Read;
        o->fd = fd; o->buf = buf; o->len = len; o->offset = offset;
    }

    // Готує sync write-операцію в базовому IoOp.
    void prep_write(IoOp* op, file_handle_t fd,
        const void* buf, size_t len, int64_t offset) noexcept override
    {
        auto* o = static_cast<SyncMockOp*>(op);
        o->backend = this;
        o->type = SyncMockOp::Type::Write;
        o->fd = fd; o->buf = const_cast<void*>(buf);
        o->len = len; o->offset = offset;
    }

    // Готує sync fsync-операцію.
    void prep_fsync(IoOp* op, file_handle_t fd) noexcept override {
        auto* o = static_cast<SyncMockOp*>(op);
        o->backend = this;
        o->type = SyncMockOp::Type::Fsync;
        o->fd = fd; o->buf = nullptr; o->len = 0;
    }

    // Виконує синхронний seek.
    int64_t seek(file_handle_t fd, int64_t offset, int whence) noexcept override {
#ifndef _WIN32
        return (int64_t)::lseek(static_cast<int>(fd), (off_t)offset, whence);
#else
        LARGE_INTEGER li{}, out{};
        li.QuadPart = offset;
        DWORD method = (whence == SEEK_SET) ? FILE_BEGIN :
            (whence == SEEK_CUR) ? FILE_CURRENT : FILE_END;
        SetFilePointerEx(reinterpret_cast<HANDLE>(fd), li, &out, method);
        return out.QuadPart;
#endif
    }

    // Обрізає файл до нового розміру.
    bool truncate(file_handle_t fd, int64_t size) noexcept override {
#ifndef _WIN32
        return ::ftruncate(static_cast<int>(fd), (off_t)size) == 0;
#else
        if (seek(fd, size, SEEK_SET) < 0) return false;
        return SetEndOfFile(reinterpret_cast<HANDLE>(fd)) != 0;
#endif
    }

    // Повертає поточний розмір файлу.
    int64_t file_size(file_handle_t fd) noexcept override {
#ifndef _WIN32
        struct stat st {};
        return (::fstat(static_cast<int>(fd), &st) == 0) ? (int64_t)st.st_size : -1;
#else
        LARGE_INTEGER sz{};
        return GetFileSizeEx(reinterpret_cast<HANDLE>(fd), &sz)
            ? sz.QuadPart : -1;
#endif
    }

    // Єдина точка submit для делегуючого UniOp.
    void submit_op(IoOp* op) noexcept override {
        static_cast<SyncMockOp*>(op)->do_io();
    }

    // Позначає backend як синхронний для await_ready-шляху.
    bool is_sync() const noexcept override { return true; }
};

#endif // _SYNC_MOCK_BACKEND_H_