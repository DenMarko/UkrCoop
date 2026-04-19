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
// Концепт: чи є тип expected-подібним
// ============================================================
template<typename T>
concept is_expected = requires(T t) {
    t.has_value();
    t.error();
};

// ============================================================
// TryAwaiter
//
// await_suspend використовує лише promise.set_error(e) —
// не знає T caller-а, тільки E. Тому TryAwaiter<int,E> може
// використовуватись всередині task<void,E> без проблем.
// ============================================================

template<typename T, typename E>
struct TryAwaiter {
    expected<T, E> val;

    bool await_ready() noexcept { return val.has_value(); }

    template<typename Promise>
    std::coroutine_handle<> await_suspend(
        std::coroutine_handle<Promise> caller) noexcept
    {
        caller.promise().set_error(val.error());
        auto cont = caller.promise().continuation;
        return cont ? cont : std::noop_coroutine();
    }

    T await_resume() noexcept { return std::move(val).value(); }
};

template<typename E>
struct TryAwaiter<void, E> {
    expected<void, E> val;

    bool await_ready() noexcept { return val.has_value(); }

    template<typename Promise>
    std::coroutine_handle<> await_suspend(
        std::coroutine_handle<Promise> caller) noexcept
    {
        caller.promise().set_error(val.error());
        auto cont = caller.promise().continuation;
        return cont ? cont : std::noop_coroutine();
    }

    void await_resume() noexcept {}
};

// ============================================================
// task<T, E>
// ============================================================

template<typename T, typename E>
struct task {
    struct promise_type {
        std::optional<expected<T, E>> result;
        std::coroutine_handle<>      continuation = nullptr;
        bool                         early_exit = false;

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
                    if (h.promise().early_exit)
                        return std::noop_coroutine();
                    auto c = h.promise().continuation;
                    return c ? c : std::noop_coroutine();
                }
                void await_resume() noexcept {}
            };
            return fa{};
        }

        void return_value(T v) { result.emplace(expected<T, E>::ok(std::move(v))); }
        void return_value(expected<T, E> r) { result.emplace(std::move(r)); }

        // викликається з TryAwaiter — не знає T caller-а, тільки E
        void set_error(E e) noexcept {
            result.emplace(expected<T, E>::err(e));
            early_exit = true;
        }

        void unhandled_exception() noexcept {
            assert(false && "use co_return expected::err(...)");
            result.emplace(expected<T, E>::err(E{}));
        }

        // перехоплення co_await expected<U,E>:
        // by-value → копіюється при lvalue, переміщується при rvalue
        template<typename U>
        TryAwaiter<U, E> await_transform(expected<U, E> e) noexcept {
            return TryAwaiter<U, E>{ std::move(e) };
        }

        // passthrough для решти (pool.schedule(), task<>, тощо)
        template<typename A> requires (!is_expected<std::remove_cvref_t<A>>)
            A&& await_transform(A&& a) noexcept { return std::forward<A>(a); }
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

private:
    // базовий awaitable — спільний для обох operator co_await
    struct awaitable_base {
        std::coroutine_handle<promise_type> handle;

        bool await_ready() const noexcept {
            return !handle || handle.done();
        }

        std::coroutine_handle<> await_suspend(std::coroutine_handle<> c) noexcept {
            handle.promise().continuation = c;
            return handle; // symmetric transfer
        }
    };

public:
    // lvalue task: co_await t  — повертає expected<T,E> by value (копія з promise)
    // використовується коли task живе довше ніж co_await
    auto operator co_await() const& noexcept {
        struct awaitable : awaitable_base {
            expected<T, E> await_resume() {
                assert(this->handle && this->handle.promise().result.has_value());
                return *this->handle.promise().result; // копія — original залишається
            }
        };
        return awaitable{ { handle_ } };
    }

    // rvalue task: co_await std::move(t)  — переміщує результат з promise
    // використовується при передачі task як тимчасового значення
    auto operator co_await() const&& noexcept {
        struct awaitable : awaitable_base {
            expected<T, E> await_resume() {
                assert(this->handle && this->handle.promise().result.has_value());
                return std::move(*this->handle.promise().result); // move — без копій
            }
        };
        return awaitable{ { handle_ } };
    }

    template<typename U, typename F> friend struct detail_sync_task;
    friend struct sync_wait_impl;
};

// ============================================================
// task<void, E>
// ============================================================

template<typename E>
struct task<void, E> {
    struct promise_type {
        std::optional<expected<void, E>> result;
        std::coroutine_handle<>         continuation = nullptr;
        bool                            early_exit = false;

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
                    if (h.promise().early_exit)
                        return std::noop_coroutine();
                    auto c = h.promise().continuation;
                    return c ? c : std::noop_coroutine();
                }
                void await_resume() noexcept {}
            };
            return fa{};
        }

        void return_value(expected<void, E> r) { result.emplace(std::move(r)); }

        void set_error(E e) noexcept {
            result.emplace(expected<void, E>::err(e));
            early_exit = true;
        }

        void unhandled_exception() noexcept {
            assert(false && "use co_return expected::err(...)");
            result.emplace(expected<void, E>::err(E{}));
        }

        template<typename U>
        TryAwaiter<U, E> await_transform(expected<U, E> e) noexcept {
            return TryAwaiter<U, E>{ std::move(e) };
        }

        template<typename A> requires (!is_expected<std::remove_cvref_t<A>>)
            A&& await_transform(A&& a) noexcept { return std::forward<A>(a); }
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

private:
    struct awaitable_base {
        std::coroutine_handle<promise_type> handle;

        bool await_ready() const noexcept {
            return !handle || handle.done();
        }

        std::coroutine_handle<> await_suspend(std::coroutine_handle<> c) noexcept {
            handle.promise().continuation = c;
            return handle;
        }
    };

public:
    auto operator co_await() const& noexcept {
        struct awaitable : awaitable_base {
            expected<void, E> await_resume() {
                assert(this->handle && this->handle.promise().result.has_value());
                return *this->handle.promise().result;
            }
        };
        return awaitable{ { handle_ } };
    }

    auto operator co_await() const&& noexcept {
        struct awaitable : awaitable_base {
            expected<void, E> await_resume() {
                assert(this->handle && this->handle.promise().result.has_value());
                return std::move(*this->handle.promise().result);
            }
        };
        return awaitable{ { handle_ } };
    }

    template<typename U, typename F> friend struct detail_sync_task;
    friend struct sync_wait_impl;
};

#endif // _TASK_H_