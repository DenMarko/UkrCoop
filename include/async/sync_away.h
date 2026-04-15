#ifndef _SYNC_AWAY_H_
#define _SYNC_AWAY_H_

#include <semaphore>
#include <cassert>
#include "async/task.h"

// ============================================================
// detail: внутрішня корутина-обгортка
// ============================================================

template<typename T>
struct detail_sync_task {
    struct promise_type {
        std::binary_semaphore* sem = nullptr;

        detail_sync_task get_return_object() noexcept {
            return detail_sync_task{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        std::suspend_always initial_suspend() noexcept { return {}; }

        auto final_suspend() noexcept {
            struct awaiter {
                bool await_ready() noexcept { return false; }

                std::coroutine_handle<> await_suspend(
                    std::coroutine_handle<promise_type> h) noexcept
                {
                    assert(h.promise().sem && "sem not set");
                    h.promise().sem->release();
                    return std::noop_coroutine();
                }

                void await_resume() noexcept {}
            };
            return awaiter{};
        }

        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); } // помилки передаються через task
    };

    std::coroutine_handle<promise_type> handle;

    explicit detail_sync_task(std::coroutine_handle<promise_type> h) noexcept : handle(h) {}
    detail_sync_task(detail_sync_task&&) = delete; // не переміщуємо — адреса sem критична
    ~detail_sync_task() { if (handle) handle.destroy(); }
};

// ============================================================
// detail: корутини-обгортки для T і void
// ============================================================

namespace detail {

template<typename T>
detail_sync_task<T> make_sync_runner(task<T> t, T* out) {
    *out = co_await std::move(t);
}

inline detail_sync_task<void> make_sync_runner(task<void> t) {
    co_await std::move(t);
}

} // namespace detail

// ============================================================
// sync_wait — публічний API
// ============================================================

struct sync_wait_impl {}; // friend tag для task

template<typename T>
T sync_wait(task<T> t) {
    std::binary_semaphore sem{0};
    T result{};

    auto runner = detail::make_sync_runner<T>(std::move(t), &result);
    runner.handle.promise().sem = &sem;

    assert(runner.handle && !runner.handle.done());
    runner.handle.resume();

    sem.acquire();

    return result;
}

inline void sync_wait(task<void> t) {
    std::binary_semaphore sem{0};

    auto runner = detail::make_sync_runner(std::move(t));
    runner.handle.promise().sem = &sem;

    assert(runner.handle && !runner.handle.done());
    runner.handle.resume();

    sem.acquire();
}
#endif // _SYNC_AWAY_H_