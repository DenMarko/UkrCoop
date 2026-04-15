#ifndef _TASK_H_
#define _TASK_H_

#include <coroutine>
#include <optional>
#include <exception>
#include <utility>
#include <cassert>

template<typename T = void>
struct task;

// ============================================================
// task<T>
// ============================================================

template<typename T>
struct task {
    struct promise_type {
        std::optional<T>        result;
        bool                    failed = false;
        std::coroutine_handle<> continuation = nullptr;

        task get_return_object() noexcept {
            return task{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_always initial_suspend() noexcept { return {}; }

        auto final_suspend() noexcept {
            struct final_awaiter {
                bool await_ready() noexcept { return false; }

                std::coroutine_handle<> await_suspend(
                    std::coroutine_handle<promise_type> h) noexcept
                {
                    auto cont = h.promise().continuation;
                    return cont ? cont : std::noop_coroutine();
                }

                void await_resume() noexcept {}
            };
            return final_awaiter{};
        }

        void return_value(T v) { result = std::move(v); }

        void unhandled_exception() noexcept {
            failed = true;
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

private:
    handle_type handle_ = nullptr;

public:
    explicit task(handle_type h) noexcept : handle_(h) {}
    task(task&& o) noexcept : handle_(std::exchange(o.handle_, {})) {}
    task(const task&) = delete;
    task& operator=(const task&) = delete;
    task& operator=(task&& o) noexcept {
        if (this != &o) {
            if (handle_) handle_.destroy();
            handle_ = std::exchange(o.handle_, {});
        }
        return *this;
    }
    ~task() { if (handle_) handle_.destroy(); }

    bool is_done() const noexcept { return !handle_ || handle_.done(); }

    // co_await
    bool await_ready() noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) noexcept {
        handle_.promise().continuation = caller;
        return handle_; // symmetric transfer
    }

    T await_resume() {
        assert(handle_ && "await_resume on empty task");

        auto& p = handle_.promise();

        assert(!p.failed && "coroutine failed");

        return std::move(*p.result);
    }

    // тільки sync_wait має прямий доступ до handle
    template<typename U> friend struct detail_sync_task;
    friend struct sync_wait_impl;
};

// ============================================================
// task<void>
// ============================================================

template<>
struct task<void> {
    struct promise_type {
        bool                    failed = false;
        std::coroutine_handle<> continuation = nullptr;

        task get_return_object() noexcept {
            return task{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_always initial_suspend() noexcept { return {}; }

        auto final_suspend() noexcept {
            struct final_awaiter {
                bool await_ready() noexcept { return false; }

                std::coroutine_handle<> await_suspend(
                    std::coroutine_handle<promise_type> h) noexcept
                {
                    auto cont = h.promise().continuation;
                    return cont ? cont : std::noop_coroutine();
                }

                void await_resume() noexcept {}
            };
            return final_awaiter{};
        }

        void return_void() noexcept {}

        void unhandled_exception() noexcept {
            failed = true;
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

private:
    handle_type handle_ = nullptr;

public:
    explicit task(handle_type h) noexcept : handle_(h) {}
    task(task&& o) noexcept : handle_(std::exchange(o.handle_, {})) {}
    task(const task&) = delete;
    task& operator=(const task&) = delete;
    task& operator=(task&& o) noexcept {
        if (this != &o) {
            if (handle_) handle_.destroy();
            handle_ = std::exchange(o.handle_, {});
        }
        return *this;
    }
    ~task() { if (handle_) handle_.destroy(); }

    bool is_done() const noexcept { return !handle_ || handle_.done(); }

    bool await_ready() noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) noexcept {
        handle_.promise().continuation = caller;
        return handle_;
    }

    void await_resume() {
        assert(handle_ && "await_resume on empty task");
        assert(!handle_.promise().failed && "coroutine failed");
    }

    template<typename U> friend struct detail_sync_task;
    friend struct sync_wait_impl;
};

#endif // _TASK_H_