#ifndef _BL_THREAD_HPP_FILE_
#define _BL_THREAD_HPP_FILE_
#include <atomic>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <future>

namespace Boundless {
class timer {
   private:
    using namespace std::chrono;
    time_point<high_resolution_clock> start, end;
    duration<uint64_t, std::nano> delta;

   public:
    timer() {}
    ~timer() {}
    void begin() { start = high_resolution_clock::now(); }
    void end() {
        end = high_resolution_clock::now();
        dalta = end - start;
    }
    friend std::ostream& operator<<(std::ostream& s, timer& c) {
        s << c.delta;
        return s;
    }
};
template <typename T>
class threadsafe_queue {
   private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

   public:
    threadsafe_queue() {}
    threadsafe_queue(threadsafe_queue const& other) {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }
    ~threadsafe_queue() {
        std::lock_guard<std::mutex> lk(mut);
        data_cond.notify_all();
    }
    void push(T new_val) {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_val);
        data_cond.notify_one();
    }
    void muti_push(T new_val, unsigned int count) {
        std::lock_guard<std::mutex> lk(mut);
        for (unsigned int i = 0; i < count; i++) {
            data_queue.push(new_val);
        }
        data_cond.notify_all();
    }
    void wait_and_pop(T& val) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        val = data_queue.front();
        data_queue.pop();
    }
    bool try_pop(T& val) {
        std::unique_lock<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;
        val = data_queue.front();
        data_queue.pop();
        return true;
    }
    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};
const unsigned int default_thread_count = 4;
class thread_pool {
   private:
    std::atomic_bool shut_down;
    threadsafe_queue<std::function<void()>> work_queue;
    std::vector<std::thread> threads;
    void worker_function() {
        std::function<void()> task;
        while (!shut_down) {
            work_queue.wait_and_pop(task);
            task();
        }
        while (work_queue.try_pop(task)) {
            task();
        }
    }
   public:
    thread_pool() : shut_down(false) {
        unsigned int thread_count = std::thread::hardware_concurrency();
        if (thread_count == 0)
            thread_count = default_thread_count;
        threads.reserve(thread_count);
        try {
            for (unsigned int i = 0; i < thread_count; ++i) {
                threads.emplace_back(&thread_pool::worker_function, this);
            }
        } catch (...) {
            shut_down = true;
            throw std::runtime_error("Thread pool construct error");
        }
    }
    ~thread_pool() {
        shut_down = true;
        for (std::thread& th : threads) {
            if (th.joinable()) {
                th.join();
            }
        }
    }
    void shutdown() {
        shut_down = true;
        for (std::thread& th : threads) {
            if (th.joinable()) {
                th.join();
            }
        }
    }
    void submit(std::function<void()> call) { work_queue.push(call); }
    void muti_submit(std::function<void()> call,unsigned int count) { work_queue.muti_push(call,count); }
    template <typename task_function, typename... arguments>
    auto submit(task_function &&f, arguments &&...args) -> std::future<decltype(f(args...))>
    {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<task_function>(f), std::forward<arguments>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        std::function<void()> warpper_func = [task_ptr]()
        { (*task_ptr)(); };
        work_queue.push(warpper_func);
        return task_ptr->get_future();
    }
    template <typename task_function, typename... arguments>
    auto muti_submit(task_function &&f, arguments &&...args) -> std::future<decltype(f(args...))>
    {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<task_function>(f), std::forward<arguments>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        std::function<void()> warpper_func = [task_ptr]()
        { (*task_ptr)(); };
        work_queue.muti_push(warpper_func);
        return task_ptr->get_future();
    }
};

}  // namespace Boundless
#endif  //!_BL_THREAD_HPP_FILE_