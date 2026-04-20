#ifndef _ASYNC_IO_ASYNC_FILE_H_
#define _ASYNC_IO_ASYNC_FILE_H_

#include <cstdio>
#include <string>
#include "sh_string.h"
#include "IoBackend.h"
#include "../task.h"
#include "../error.h"
#include "../expected.h"

#ifndef _WIN32
#   include <fcntl.h>
#   include <unistd.h>
#   include "IouringBackend.h"
#else
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include "IocpBackend.h"
#endif

// ============================================================
// OpenFlags
// ============================================================
enum class OpenFlags : unsigned {
    Read = 1 << 0,
    Write = 1 << 1,
    Append = 1 << 2,
    Trunc = 1 << 3,
    Create = 1 << 4,
};

inline OpenFlags operator|(OpenFlags a, OpenFlags b) {
    return static_cast<OpenFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
inline bool flag_set(OpenFlags flags, OpenFlags bit) noexcept {
    return (static_cast<unsigned>(flags) & static_cast<unsigned>(bit)) != 0;
}

// ============================================================
// AsyncFile<Op>
//
// Op = конкретний тип IoOp для поточної платформи:
//   Linux prod:   IouringOp
//   Windows prod: IocpOp
//   Тести/fallback: SyncMockOp
//
// template параметр дозволяє уникнути macro і зберігає
// zero-overhead: Op живе на стеку корутини, без heap алокацій.
// ============================================================
template<typename Op>
class AsyncFile {
private:

    static constexpr file_handle_t INVALID_HANDLE = static_cast<file_handle_t>(-1);

public:
    explicit AsyncFile(IoBackend& backend) noexcept
        : backend_(backend) {}

    ~AsyncFile() { close(); }

    AsyncFile(const AsyncFile&) = delete;
    AsyncFile& operator=(const AsyncFile&) = delete;

    AsyncFile(AsyncFile&& other) noexcept
        : backend_(other.backend_), handle_(other.handle_), pos_(other.pos_)
    {
        other.handle_ = INVALID_HANDLE;
        other.pos_ = 0;
    }

    bool is_open() const noexcept { return handle_ != INVALID_HANDLE; }

    // ── open ──────────────────────────────────────────────
    task<void> open(const char* path, OpenFlags flags)
    {
        if(is_open()) close();

        int native_flags = to_native_flags(flags);
        handle_ = backend_.open(path, native_flags, 0644);
        pos_ = 0;

        if(handle_ == INVALID_HANDLE)
        {
            co_return expected<void>::err(error_code::file_open_failed);
        }

        if(flag_set(flags, OpenFlags::Append))
        {
            int64_t end = backend_.seek(handle_, 0, SEEK_END);
            if(end < 0)
            {
                close();
                co_return expected<void>::err(error_code::file_open_failed);
            }
            pos_ = end;
        }

        co_return expected<void>::ok();
    }

    void close() noexcept
    {
        if(is_open())
        {
            backend_.close(handle_);
            handle_ = INVALID_HANDLE;
            pos_ = 0;
        }
    }

    // ── read ──────────────────────────────────────────────
    task<size_t> read(void*buffer, size_t len)
    {
        if(!is_open())
            co_return expected<size_t>::err(error_code::not_initialized);

        Op op;
        backend_.prep_read(&op, handle_, buffer, len, pos_);
        IoResult res = co_await op;

        if(!res.ok())
            co_return expected<size_t>::err(error_code::request_failed);

        pos_ += res.bytes;
        co_return static_cast<size_t>(res.bytes);
    }

    task<size_t> read_exact(void* buffer, size_t len)
    {
        size_t      total = 0;
        uint8_t*    dst = static_cast<uint8_t*>(buffer);

        while(total < len)
        {
            auto r = co_await read(dst + total, len - total);
            if(!r) co_return expected<size_t>::err(r.error());
            if(*r == 0) break; // EOF
            total += *r;
        }

        co_return total;
    }

    // ── write ─────────────────────────────────────────────
    task<size_t> write(const void* buffer, size_t len)
    {
        if(!is_open())
            co_return expected<size_t>::err(error_code::not_initialized);

        Op op;
        backend_.prep_write(&op, handle_, buffer, len, pos_);
        IoResult res = co_await op;

        if(!res.ok())
            co_return expected<size_t>::err(error_code::write_failed);

        pos_ += res.bytes;
        co_return static_cast<size_t>(res.bytes);
    }

    task<void> write_all(const void* buffer, size_t len)
    {
        size_t      total = 0;
        const uint8_t* src = static_cast<const uint8_t*>(buffer);

        while(total < len)
        {
            auto r = co_await write(src + total, len - total);
            if(!r) co_return expected<void>::err(r.error());
            total += *r;
        }

        co_return expected<void>::ok();
    }

    task<void> write_str(const char* str)
    {
        return write_all(str, cstrlen(str));
    }

    task<void> write_str(const char* str, size_t len)
    {
        return write_all(str, len);
    }

    task<void> write_str(const std::string& str)
    {
        return write_all(str.data(), str.size());
    }

    task<void> write_str(const SourceHook::String &str)
    {
        return write_all(str.c_str(), str.size());
    }

    // ── seek / tell ───────────────────────────────────────
    task<int64_t> seek(int64_t offset, int whence = SEEK_SET) noexcept
    {
        if(!is_open())
            co_return expected<int64_t>::err(error_code::not_initialized);

        int64_t new_pos = backend_.seek(handle_, offset, whence);
        if(new_pos < 0)
            co_return expected<int64_t>::err(error_code::request_failed);

        pos_ = new_pos;
        co_return expected<int64_t>::ok(new_pos);
    }

    int64_t tell() const noexcept { return pos_; }

    // ── flush / fsync ─────────────────────────────────────
    task<void> flush()
    {
        if(!is_open())
            co_return expected<void>::err(error_code::not_initialized);

        Op op;
        backend_.prep_fsync(&op, handle_);
        IoResult res = co_await op;

        if(!res.ok())
            co_return expected<void>::err(error_code::request_failed);

        co_return expected<void>::ok();
    }

    // ── size / truncate ───────────────────────────────────
    task<int64_t> size() noexcept
    {
        if(!is_open())
            co_return expected<int64_t>::err(error_code::not_initialized);

        int64_t sz = backend_.file_size(handle_);
        if(sz < 0)
            co_return expected<int64_t>::err(error_code::request_failed);

        co_return expected<int64_t>::ok(sz);
    }

    task<void> truncate(int64_t size) noexcept
    {
        if(!is_open())
            co_return expected<void>::err(error_code::not_initialized);

        if(!backend_.truncate(handle_, size))
            co_return expected<void>::err(error_code::request_failed);

        if(pos_ > size) pos_ = size; // якщо поточна позиція за межами нового розміру, переміщаємо її в кінець

        co_return expected<void>::ok();
    }

    expected<void> truncate_at_pos() noexcept
    {
        return truncate(pos_);
    }

private:
    static int to_native_flags(OpenFlags f) noexcept
    {
        int flags = 0;
        bool rd = flag_set(f, OpenFlags::Read);
        bool wr = flag_set(f, OpenFlags::Write) || flag_set(f, OpenFlags::Append);

        if(rd && wr) flags = O_RDWR;
        else if(wr) flags = O_WRONLY;
        else flags = O_RDONLY;

        if(flag_set(f, OpenFlags::Append)) flags |= O_APPEND;
        if(flag_set(f, OpenFlags::Trunc)) flags |= O_TRUNC;
        if(flag_set(f, OpenFlags::Create)) flags |= O_CREAT;

        return flags;
    }

    static size_t cstrlen(const char* str) noexcept
    {
        const char* s = str;
        while (*s) ++s;
        return static_cast<size_t>(s - str);
    }

    IoBackend &backend_;
    file_handle_t handle_ = INVALID_HANDLE;
    int64_t pos_ = 0;
};

#ifndef _WIN32
using AsyncFileOP = AsyncFile<IouringOp>;
#else
using AsyncFileOP = AsyncFile<IocpOp>;
#endif

// ── Файлові утиліти (не залежать від template параметра Op) ─
namespace async_fs {
    inline bool exists(const char* path) noexcept {
#ifdef _WIN32
        return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
#else
        return ::access(path, F_OK) == 0;
#endif
    }

    inline bool remove(const char* path) noexcept {
#ifdef _WIN32
        return DeleteFileA(path) != 0;
#else
        return ::unlink(path) == 0;
#endif
    }

    inline bool rename(const char* from, const char* to) noexcept {
#ifdef _WIN32
        return MoveFileExA(from, to, MOVEFILE_REPLACE_EXISTING) != 0;
#else
        return std::rename(from, to) == 0;
#endif
    }
} // namespace async_fs


#endif // _ASYNC_IO_ASYNC_FILE_H_