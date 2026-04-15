#ifndef _FIRE_AND_FORGET_H_
#define _FIRE_AND_FORGET_H_

#include <coroutine>
#include <exception>

struct fire_and_forget {
    struct promise_type {
        fire_and_forget get_return_object() noexcept { return {}; }

        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend()   noexcept { return {}; }

        void return_void() noexcept {}

        // виняток = terminate, як в WinRT
        // в debug можна замінити на assert з повідомленням
        void unhandled_exception() noexcept {
            std::terminate();
        }
    };
};

#endif // _FIRE_AND_FORGET_H_