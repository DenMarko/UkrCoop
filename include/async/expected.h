#ifndef _ASYNC_EXPECTED_H_
#define _ASYNC_EXPECTED_H_

#include <variant>
#include <cassert>
#include <utility>
#include "error.h"

// expected<T, E=error_code>
// Проброс помилки відбувається через await_transform в task::promise_type
// (див. task.hpp → TryAwaiter). operator co_await тут не потрібен.

template<typename T, typename E = error_code>
struct expected {
    static expected ok(T val) {
        expected r;
        r.storage_.template emplace<1>(std::move(val));
        return r;
    }

    static expected err(E e = E{}) {
        expected r;
        r.storage_.template emplace<2>(e);
        return r;
    }

    bool has_value() const noexcept { return storage_.index() == 1; }
    explicit operator bool() const noexcept { return has_value(); }

    // value() — три перевантаження для &, const&, &&
    T& value()& {
        assert(has_value() && "expected: value on error");
        return std::get<1>(storage_);
    }
    const T& value() const& {
        assert(has_value() && "expected: value on error");
        return std::get<1>(storage_);
    }
    T&& value()&& {
        assert(has_value() && "expected: value on error");
        return std::move(std::get<1>(storage_));
    }

    T& operator*()& { return value(); }
    const T& operator*() const& { return value(); }
    T&& operator*()&& { return std::move(*this).value(); }

    E error() const noexcept {
        return has_value() ? E{} : std::get<2>(storage_);
    }

    T value_or(T fallback) const& {
        return has_value() ? std::get<1>(storage_) : std::move(fallback);
    }

    template<typename F>
    auto map(F&& f) -> expected<decltype(f(std::declval<T>())), E> {
        using U = decltype(f(std::declval<T>()));
        if (has_value()) return expected<U, E>::ok(f(std::get<1>(storage_)));
        return expected<U, E>::err(std::get<2>(storage_));
    }

    template<typename F>
    auto and_then(F&& f) -> decltype(f(std::declval<T>())) {
        if (has_value()) return f(std::get<1>(storage_));
        using Ret = decltype(f(std::declval<T>()));
        return Ret::err(std::get<2>(storage_));
    }

private:
    expected() {}
    std::variant<std::monostate, T, E> storage_;
};

// --------------------------------------------------------
// expected<void, E>
// --------------------------------------------------------
template<typename E>
struct expected<void, E> {
    static expected ok() { expected r; r.ok_ = true;  return r; }
    static expected err(E e = E{}) { expected r; r.ok_ = false; r.e_ = e; return r; }

    bool has_value() const noexcept { return ok_; }
    explicit operator bool() const noexcept { return ok_; }
    E error() const noexcept { return e_; }

private:
    expected() {}
    bool ok_ = false;
    E    e_ = E{};
};

using result_void = expected<void, error_code>;

#endif // _ASYNC_EXPECTED_H_