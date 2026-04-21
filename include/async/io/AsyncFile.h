#ifndef _ASYNC_IO_ASYNC_FILE_H_
#define _ASYNC_IO_ASYNC_FILE_H_

#include <cstdio>
#include <string>
#include "IoBackend.h"
#include "../task.h"
#include "../error.h"
#include "../expected.h"

#ifndef _WIN32
#  include <fcntl.h>
#  include <unistd.h>
#else
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
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
    return static_cast<OpenFlags>(
        static_cast<unsigned>(a) | static_cast<unsigned>(b));
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
public:
    // Прив'язує файлову обгортку до конкретного backend-а.
    explicit AsyncFile(IoBackend& backend) noexcept
        : backend_(backend) {
    }

    // Закриває файл при знищенні обгортки.
    ~AsyncFile() { close(); }

    AsyncFile(const AsyncFile&) = delete;
    AsyncFile& operator=(const AsyncFile&) = delete;

    // Переміщує відкритий дескриптор і поточну позицію без копіювання.
    AsyncFile(AsyncFile&& o) noexcept
        : backend_(o.backend_), fd_(o.fd_), pos_(o.pos_)
    {
        o.fd_ = INVALID_FD;
        o.pos_ = 0;
    }

    // Перевіряє, чи файл зараз відкритий.
    bool is_open() const noexcept { return fd_ != INVALID_FD; }

    // ── open ──────────────────────────────────────────────
    // Відкриває файл і ініціалізує поточну позицію доступу.
    task<void> open(const char* path, OpenFlags flags) {
        if (is_open()) close();

        int native = to_native_flags(flags);
        fd_ = backend_.open(path, native, 0644);
        pos_ = 0;

        if (fd_ == INVALID_FD)
            co_return expected<void>::err(error_code::file_open_failed);

        if (flag_set(flags, OpenFlags::Append)) {
            int64_t end = backend_.seek(fd_, 0, SEEK_END);
            if (end < 0) {
                close();
                co_return expected<void>::err(error_code::file_open_failed);
            }
            pos_ = end;
        }

        co_return expected<void>::ok();
    }

    // Закриває відкритий файл і скидає внутрішній стан.
    void close() noexcept {
        if (is_open()) {
            backend_.close(fd_);
            fd_ = INVALID_FD;
            pos_ = 0;
        }
    }

    // ── read ──────────────────────────────────────────────
    // Виконує одну асинхронну операцію читання з поточної позиції.
    task<size_t> read(void* buf, size_t len) {
        if (!is_open())
            co_return expected<size_t>::err(error_code::not_initialized);

        Op op;
        backend_.prep_read(&op, fd_, buf, len, pos_);
        IoResult r = co_await op;

        if (!r.ok())
            co_return expected<size_t>::err(error_code::request_failed);

        pos_ += r.bytes;
        co_return static_cast<size_t>(r.bytes);
    }

    // Читає буфер повністю або зупиняється на EOF/помилці.
    task<size_t> read_exact(void* buf, size_t len) {
        size_t   total = 0;
        uint8_t* dst = static_cast<uint8_t*>(buf);

        while (total < len) {
            auto r = co_await read(dst + total, len - total);
            if (!r)   co_return expected<size_t>::err(r.error());
            if (*r == 0) break;
            total += *r;
        }

        co_return total;
    }

    // ── write ─────────────────────────────────────────────
    // Виконує одну асинхронну операцію запису з поточної позиції.
    task<size_t> write(const void* buf, size_t len) {
        if (!is_open())
            co_return expected<size_t>::err(error_code::not_initialized);

        Op op;
        backend_.prep_write(&op, fd_, buf, len, pos_);
        IoResult r = co_await op;

        if (!r.ok())
            co_return expected<size_t>::err(error_code::write_failed);

        pos_ += r.bytes;
        co_return static_cast<size_t>(r.bytes);
    }

    // Повторює write, поки весь буфер не буде записано.
    task<void> write_all(const void* buf, size_t len) {
        size_t         written = 0;
        const uint8_t* src = static_cast<const uint8_t*>(buf);

        while (written < len) {
            auto r = co_await write(src + written, len - written);
            if (!r) co_return expected<void>::err(r.error());
            written += *r;
        }

        co_return expected<void>::ok();
    }

    // Записує C-рядок без термінального нуля.
    task<void> write_str(const char* s) {
        return write_all(s, cstrlen(s));
    }

    // Записує рядок заданої довжини.
    task<void> write_str(const char* s, size_t len) {
        return write_all(s, len);
    }

    // Записує owning-копію std::string, безпечну для async lifetime.
    task<void> write_str(std::string s) {
        return write_all(s.data(), s.size());
    }

    // ── seek / tell ───────────────────────────────────────
    // Переміщує файлову позицію через backend-specific seek.
    expected<int64_t> seek(int64_t offset, int whence = SEEK_SET) noexcept {
        if (!is_open())
            return expected<int64_t>::err(error_code::not_initialized);

        int64_t pos = backend_.seek(fd_, offset, whence);
        if (pos < 0)
            return expected<int64_t>::err(error_code::request_failed);

        pos_ = pos;
        return expected<int64_t>::ok(pos);
    }

    // Повертає кешовану поточну позицію.
    int64_t tell() const noexcept { return pos_; }

    // ── flush / fsync ─────────────────────────────────────
    // Синхронізує буфери файлу з диском.
    task<void> flush() {
        if (!is_open())
            co_return expected<void>::err(error_code::not_initialized);

        Op op;
        backend_.prep_fsync(&op, fd_);
        IoResult r = co_await op;

        if (!r.ok())
            co_return expected<void>::err(error_code::write_failed);

        co_return expected<void>::ok();
    }

    // ── size / truncate ───────────────────────────────────
    // Повертає поточний розмір файлу.
    expected<int64_t> size() noexcept {
        if (!is_open())
            return expected<int64_t>::err(error_code::not_initialized);

        int64_t s = backend_.file_size(fd_);
        if (s < 0)
            return expected<int64_t>::err(error_code::request_failed);

        return expected<int64_t>::ok(s);
    }

    // Обрізає файл до нового розміру.
    expected<void> truncate(int64_t new_size) noexcept {
        if (!is_open())
            return expected<void>::err(error_code::not_initialized);

        if (!backend_.truncate(fd_, new_size))
            return expected<void>::err(error_code::write_failed);

        if (pos_ > new_size) pos_ = new_size;
        return expected<void>::ok();
    }

    // Обрізає файл до поточної позиції.
    expected<void> truncate_at_pos() noexcept {
        return truncate(pos_);
    }



private:
    // Перекладає кросплатформні прапори у native open flags.
    static int to_native_flags(OpenFlags f) noexcept {
        int flags = 0;
        bool rd = flag_set(f, OpenFlags::Read);
        bool wr = flag_set(f, OpenFlags::Write)
            || flag_set(f, OpenFlags::Append);

        if (rd && wr) flags = O_RDWR;
        else if (wr)       flags = O_WRONLY;
        else               flags = O_RDONLY;

        if (flag_set(f, OpenFlags::Create)) flags |= O_CREAT;
        if (flag_set(f, OpenFlags::Trunc))  flags |= O_TRUNC;
        if (flag_set(f, OpenFlags::Append)) flags |= O_APPEND;

        return flags;
    }

    // Рахує довжину C-рядка без залежності від libc strlen.
    static size_t cstrlen(const char* s) noexcept {
        const char* p = s; while (*p) ++p;
        return static_cast<size_t>(p - s);
    }

    static constexpr file_handle_t INVALID_FD = static_cast<file_handle_t>(-1);

    IoBackend& backend_;
    file_handle_t fd_ = INVALID_FD;
    int64_t    pos_ = 0;
};

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