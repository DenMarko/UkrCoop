#ifndef _SYNC_AWAY_H_
#define _SYNC_AWAY_H_
#include <semaphore>
#include <cassert>
#include "task.h"

template<typename T, typename E>
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
                    assert(h.promise().sem);
                    h.promise().sem->release();
                    return std::noop_coroutine();
                }
                void await_resume() noexcept {}
            };
            return awaiter{};
        }

        void return_void() noexcept {}
        void unhandled_exception() noexcept { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;
    explicit detail_sync_task(std::coroutine_handle<promise_type> h) noexcept : handle(h) {}
    detail_sync_task(detail_sync_task&&) = delete;
    ~detail_sync_task() { if (handle) handle.destroy(); }
};

namespace detail {

    template<typename T, typename E>
    detail_sync_task<T, E> make_sync_runner(task<T, E> t, expected<T, E>* out) {
        *out = co_await std::move(t);
    }

    template<typename E>
    detail_sync_task<void, E> make_sync_runner(task<void, E> t, expected<void, E>* out) {
        *out = co_await std::move(t);
    }

} // namespace detail

struct sync_wait_impl {};

template<typename T, typename E = error_code>
expected<T, E> sync_wait(task<T, E> t) {
    std::binary_semaphore sem{ 0 };
    expected<T, E> result = expected<T, E>::err(E{});

    auto runner = detail::make_sync_runner<T, E>(std::move(t), &result);
    runner.handle.promise().sem = &sem;
    assert(runner.handle && !runner.handle.done());
    runner.handle.resume();
    sem.acquire();

    return result;
}

template<typename E = error_code>
expected<void, E> sync_wait(task<void, E> t) {
    std::binary_semaphore sem{ 0 };
    expected<void, E> result = expected<void, E>::err(E{});

    auto runner = detail::make_sync_runner<E>(std::move(t), &result);
    runner.handle.promise().sem = &sem;
    assert(runner.handle && !runner.handle.done());
    runner.handle.resume();
    sem.acquire();

    return result;
}

#endif // _SYNC_AWAY_H_