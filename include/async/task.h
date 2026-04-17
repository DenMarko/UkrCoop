#ifndef _TASK_H_
#define _TASK_H_

#pragma once
#include <coroutine>
#include <optional>
#include <utility>
#include <cassert>
#include "expected.h"

template<typename T = void, typename E = error_code>
struct task;

// ============================================================
// task<T, E>
// ============================================================

template<typename T, typename E>
struct [[nodiscard]] task {
    struct promise_type {
        std::optional<expected<T, E>> result;
        std::coroutine_handle<>      continuation = nullptr;

        task get_return_object() noexcept {
            return task{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_always initial_suspend() noexcept { return {}; }

        auto final_suspend() noexcept {
            struct fa {
                bool await_ready() noexcept { return false; }
                std::coroutine_handle<> await_suspend(
                    std::coroutine_handle<promise_type> h) noexcept
                {
                    auto c = h.promise().continuation;
                    return c ? c : std::noop_coroutine();
                }
                void await_resume() noexcept {}
            };
            return fa{};
        }

        // co_return T{} → ok
        void return_value(T v) {
            result.emplace(expected<T, E>::ok(std::move(v)));
        }

        // co_return expected<T,E>{...} → передаємо як є
        void return_value(expected<T, E> r) {
            result.emplace(std::move(r));
        }

        void unhandled_exception() noexcept {
            assert(false && "unhandled exception — use co_return expected::err(...)");
            result.emplace(expected<T, E>::err(E{}));
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
        if (this != &o) { if (handle_) handle_.destroy(); handle_ = std::exchange(o.handle_, {}); }
        return *this;
    }
    ~task() { if (handle_) handle_.destroy(); }

    bool is_done() const noexcept { return !handle_ || handle_.done(); }

    bool await_ready() noexcept { return false; }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> c) noexcept {
        handle_.promise().continuation = c;
        return handle_;
    }
    expected<T, E> await_resume() {
        assert(handle_ && handle_.promise().result.has_value());
        return std::move(*handle_.promise().result);
    }

    template<typename U, typename F> friend struct detail_sync_task;
    friend struct sync_wait_impl;
};

// ============================================================
// task<void, E>
// ============================================================

template<typename E>
struct [[nodiscard]] task<void, E> {
    struct promise_type {
        std::optional<expected<void, E>> result;
        std::coroutine_handle<>         continuation = nullptr;

        task get_return_object() noexcept {
            return task{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_always initial_suspend() noexcept { return {}; }

        auto final_suspend() noexcept {
            struct fa {
                bool await_ready() noexcept { return false; }
                std::coroutine_handle<> await_suspend(
                    std::coroutine_handle<promise_type> h) noexcept
                {
                    auto c = h.promise().continuation;
                    return c ? c : std::noop_coroutine();
                }
                void await_resume() noexcept {}
            };
            return fa{};
        }

        // co_return expected<void,E>{...}
        void return_value(expected<void, E> r) {
            result.emplace(std::move(r));
        }

        void unhandled_exception() noexcept {
            assert(false && "unhandled exception — use co_return expected::err(...)");
            result.emplace(expected<void, E>::err(E{}));
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
        if (this != &o)
        {
            if (handle_)
                handle_.destroy();

            handle_ = std::exchange(o.handle_, {});
        }
        return *this;
    }
    ~task() { if (handle_) handle_.destroy(); }

    bool is_done() const noexcept { return !handle_ || handle_.done(); }

    bool await_ready() noexcept { return false; }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> c) noexcept {
        handle_.promise().continuation = c;
        return handle_;
    }
    expected<void, E> await_resume() {
        assert(handle_ && handle_.promise().result.has_value());
        return std::move(*handle_.promise().result);
    }

    template<typename U, typename F> friend struct detail_sync_task;
    friend struct sync_wait_impl;
};
#endif // _TASK_H_