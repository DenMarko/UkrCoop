#ifndef _ASYNC_IO_ASYNC_FILE_AUTO_H_
#define _ASYNC_IO_ASYNC_FILE_AUTO_H_

#include "IoBackend.h"
#include "SyncMockBackend.h"

#ifndef _WIN32
#  include "IouringBackend.h"
#else
#  include "IocpBackend.h"
#endif

#include "AsyncFile.h"

#include <atomic>
#include <memory>

// Єдиний Op-тип, який делегує submit конкретному backend-у.
struct UniOp final : IoOp {
protected:
    // Делегує submit вибраному backend-у.
    void submit() noexcept override {
        if(backend) backend->submit_op(this);
    }

    // Для sync backend-ів виконує I/O ще на етапі await_ready.
    void submit_if_sync() noexcept override {
        if (backend && backend->is_sync())
            backend->submit_op(this);
    }
};

// Singleton-контекст, який автоматично обирає backend.
class IoContext {
public:
    // Явно запускає singleton-backend.
    static void init() { instance().ensure_started(); }
    // Явно зупиняє singleton-backend.
    static void shutdown() { instance().do_stop(); }

    // Повертає активний backend із lazy-start поведінкою.
    static IoBackend& backend() { return instance().get_backend(); }
    // Повертає ім'я вибраного backend-а для логів.
    static const char* backend_name() noexcept { return instance().name_; }

private:
    // Обирає platform-specific backend або fallback при першому доступі.
    IoContext() {
#ifdef _WIN32
        backend_ = std::make_unique<IocpBackend>();
        name_ = "iocp";
#else
        auto uring = std::make_unique<IouringBackend>();
        if (uring->is_available()) {
            backend_ = std::move(uring);
            name_ = "io_uring";
        } else {
            backend_ = std::make_unique<SyncMockBackend>();
            name_ = "sync_fallback";
        }
#endif
    }

    // Гарантує коректну зупинку backend-а при завершенні процесу.
    ~IoContext() { do_stop(); }
    IoContext(const IoContext&) = delete;
    IoContext& operator=(const IoContext&) = delete;

    // Повертає єдиний інстанс контексту в процесі.
    static IoContext& instance() {
        static IoContext ctx;
        return ctx;
    }

    // Запускає backend лише один раз навіть при гонках.
    void ensure_started() {
        bool expected = false;
        if (started_.compare_exchange_strong(
            expected, true, std::memory_order_acq_rel))
        {
            backend_->start();
        }
    }

    // Зупиняє backend лише якщо він був запущений.
    void do_stop() {
        if (started_.exchange(false, std::memory_order_acq_rel))
            backend_->stop();
    }

    // Основна точка доступу до backend-а з lazy initialization.
    IoBackend& get_backend() {
        ensure_started();
        return *backend_;
    }

    std::unique_ptr<IoBackend> backend_;
    std::atomic<bool> started_{ false };
    const char* name_ = "unknown";
};

// AsyncFile з автоматичним backend-ом.
class AsyncFileAuto : public AsyncFile<UniOp> {
public:
    // Створює AsyncFile з автоматично вибраним backend-ом.
    AsyncFileAuto()
        : AsyncFile<UniOp>(IoContext::backend()) {}
};

#endif // _ASYNC_IO_ASYNC_FILE_AUTO_H_