#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <coroutine>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <cassert>

class thread_pool {
    std::vector<std::thread>          workers_;
    std::queue<std::coroutine_handle<>> queue_; // зберігаємо handle напряму — без std::function
    std::mutex                        mtx_;
    std::condition_variable           cv_;
    bool                              stop_ = false;

public:
    explicit thread_pool(size_t n = std::thread::hardware_concurrency()) {
        assert(n > 0);
        workers_.reserve(n);
        for (size_t i = 0; i < n; i++) {
            workers_.emplace_back([this] { worker_loop(); });
        }
    }

    ~thread_pool() {
        {
            std::lock_guard lock(mtx_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& w : workers_) w.join();
    }

    // не копіюємо і не переміщуємо — pool живе як singleton або member
    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;

    size_t thread_count() const noexcept { return workers_.size(); }

    // co_await pool.schedule() — аналог winrt::resume_background()
    auto schedule() noexcept {
        struct awaitable {
            thread_pool& pool;

            bool await_ready() noexcept { return false; }

            void await_suspend(std::coroutine_handle<> h) noexcept {
                pool.post(h);
            }

            void await_resume() noexcept {}
        };
        return awaitable{ *this };
    }

private:
    void post(std::coroutine_handle<> h) noexcept {
        assert(h && "posting null handle");
        {
            std::lock_guard lock(mtx_);
            if (stop_) {
                // pool зупиняється — виконуємо на поточному потоці
                // щоб корутина могла завершитись і звільнити пам'ять
                h.resume();
                return;
            }
            queue_.push(h);
        }
        cv_.notify_one();
    }

    void worker_loop() {
        while (true) {
            std::coroutine_handle<> h;
            {
                std::unique_lock lock(mtx_);
                cv_.wait(lock, [this] { return stop_ || !queue_.empty(); });
                if (stop_ && queue_.empty()) return;
                h = queue_.front();
                queue_.pop();
            }

            assert(h && !h.done() && "resuming invalid handle");
            h.resume();
        }
    }
};

#endif // _THREAD_POOL_H_