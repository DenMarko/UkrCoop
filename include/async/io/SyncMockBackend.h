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
protected:
    // У sync backend submit просто виконує I/O на місці.
    void submit()         noexcept override {}
    // Дозволяє await_ready завершити I/O без suspend.
    void submit_if_sync() noexcept override;
};

class SyncMockBackend final : public IoBackend {
public:
    void start() noexcept override {}
    bool is_sync() const noexcept override { return true; }

    void submit_op(IoOp* op) noexcept override { execute(op); }
    void stop()  noexcept override {}

    int open(const char* path, int flags, int mode) override {
#ifndef _WIN32
        return ::open(path, flags, mode);
#else
        DWORD access = 0, creation = OPEN_EXISTING;
        DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE;
        if (flags & 0x02 || flags & 0x01) access |= GENERIC_READ;
        if (flags & 0x01 || flags & 0x0200) access |= GENERIC_WRITE;
        if (flags & 0x200) creation = (flags & 0x40) ? CREATE_ALWAYS : TRUNCATE_EXISTING;
        else if (flags & 0x40) creation = OPEN_ALWAYS;
        HANDLE h = CreateFileA(path, access, share, nullptr,
            creation, FILE_ATTRIBUTE_NORMAL, nullptr);
        return h == INVALID_HANDLE_VALUE ? -1 : (int)(intptr_t)h;
#endif
    }

    void close(int fd) override {
#ifndef _WIN32
        ::close(fd);
#else
        CloseHandle((HANDLE)(intptr_t)fd);
#endif
    }

    void prep_read(IoOp* op, int fd,
                   void* buf, size_t len, int64_t offset) noexcept override {
        op->backend = this;
        op->type    = IoOp::Type::Read;
        op->fd = fd; op->buf = buf; op->len = len; op->offset = offset;
    }

    void prep_write(IoOp* op, int fd,
                    const void* buf, size_t len, int64_t offset) noexcept override {
        op->backend = this;
        op->type    = IoOp::Type::Write;
        op->fd = fd; op->buf = const_cast<void*>(buf);
        op->len = len; op->offset = offset;
    }

    void prep_fsync(IoOp* op, int fd) noexcept override {
        op->backend = this;
        op->type    = IoOp::Type::Fsync;
        op->fd = fd; op->buf = nullptr; op->len = 0;
    }

    // Виконати операцію синхронно — викликається з SyncMockOp::submit_if_sync()
    void execute(IoOp* op) noexcept {
        int32_t res = 0;
        switch (op->type) {
#ifndef _WIN32
            case IoOp::Type::Read: {
                ssize_t n = ::pread(op->fd, op->buf, op->len, (off_t)op->offset);
                res = (n < 0) ? -errno : (int32_t)n;
                break;
            }
            case IoOp::Type::Write: {
                ssize_t n = ::pwrite(op->fd, op->buf, op->len, (off_t)op->offset);
                res = (n < 0) ? -errno : (int32_t)n;
                break;
            }
            case IoOp::Type::Fsync:
                res = (::fsync(op->fd) == 0) ? 0 : -errno;
                break;
#else
            case IoOp::Type::Read: {
                DWORD got = 0;
                BOOL ok = ReadFile((HANDLE)(intptr_t)op->fd,
                    op->buf, (DWORD)op->len, &got, nullptr);
                res = ok ? (int32_t)got : -(int32_t)GetLastError();
                break;
            }
            case IoOp::Type::Write: {
                DWORD written = 0;
                BOOL ok = WriteFile((HANDLE)(intptr_t)op->fd,
                    op->buf, (DWORD)op->len, &written, nullptr);
                res = ok ? (int32_t)written : -(int32_t)GetLastError();
                break;
            }
            case IoOp::Type::Fsync:
                res = FlushFileBuffers((HANDLE)(intptr_t)op->fd) ? 0
                    : -(int32_t)GetLastError();
                break;
#endif
        }
        op->complete(res);
    }

    int64_t seek(int fd, int64_t o, int w) noexcept override {
#ifndef _WIN32
        return (int64_t)::lseek(fd, (off_t)o, w);
#else
        LARGE_INTEGER li{}, out{};
        li.QuadPart = o;
        DWORD m = (w==0)?FILE_BEGIN:(w==1)?FILE_CURRENT:FILE_END;
        SetFilePointerEx((HANDLE)(intptr_t)fd, li, &out, m);
        return out.QuadPart;
#endif
    }

    bool truncate(int fd, int64_t s) noexcept override {
#ifndef _WIN32
        return ::ftruncate(fd, (off_t)s) == 0;
#else
        if (seek(fd, s, 0) < 0) return false;
        return SetEndOfFile((HANDLE)(intptr_t)fd) != 0;
#endif
    }

    int64_t file_size(int fd) noexcept override {
#ifndef _WIN32
        struct stat st{};
        return ::fstat(fd, &st) == 0 ? (int64_t)st.st_size : -1;
#else
        LARGE_INTEGER sz{};
        return GetFileSizeEx((HANDLE)(intptr_t)fd, &sz) ? sz.QuadPart : -1;
#endif
    }
};

inline void SyncMockOp::submit_if_sync() noexcept {
    static_cast<SyncMockBackend*>(backend)->execute(this);
}

#endif // _SYNC_MOCK_BACKEND_H_