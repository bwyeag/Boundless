#ifndef _BL_THREADPOOL_HPP_FILE_
#define _BL_THREADPOOL_HPP_FILE_
#include <atomic>
#include <barrier>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>

namespace Boundless {
template <typename T, size_t capacity = 64>
class SyncQueue {
   private:
    std::mutex qumut;
    int front, rear;
    T arr[capacity];
    inline int inc(int a) {
        if constexpr (capacity & (capacity - 1) == 0) {
            return (a + 1) & (capacity - 1);
        } else {
            return (a + 1) % capacity;
        }
    }

   public:
    SyncQueue() {
        static_assert(capacity < 2 || capacity > 8192);
        front = rear = 0;
    }
    SyncQueue(SyncQueue&&) = delete;
    SyncQueue(const SyncQueue&) = delete;
    SyncQueue& operator=(SyncQueue&&) = delete;
    SyncQueue& operator=(const SyncQueue&) = delete;
    bool try_push(const T& val) {
        qumut.lock();
        if (inc(rear) == front) {
            qumut.unlock();
            return false;
        } else {
            arr[rear] = val;
            rear = inc(rear);
            qumut.unlock();
            return true;
        }
    }
    bool try_pop(T* ret) {
        qumut.lock();
        if (rear == front) {
            qumut.unlock();
            return false;
        } else {
            *ret = arr[front];
            front = inc(front);
            qumut.unlock();
            return true;
        }
    }
    bool empty() {
        qumut.lock();
        bool res = rear == front;
        qumut.unlock();
        return res;
    }
};

class ThreadPool {
   private:
    const size_t queue_capacity = 64;
    const size_t thread_capacity = 12;
    using TaskQueue = SyncQueue<std::function<void()>, queue_capacity>;

    TaskQueue* taskqueues;
    std::thread* threads;
    std::atomic_bool shutdown{false};

    void thread_worker(std::barrier& mb, int id) {
        std::function<void()> res;
        mb.arrive_and_wait();
        while (true) {
            if (taskqueues[id].try_pop(&res)) {
                res();
                continue;
            } else {
                for (size_t i = 0; i < id; i++) {
                    if (taskqueues[i].try_pop(&res)) {
                        res();
                        break;
                    }
                }
            }
        }
    }

   public:
    ThreadPool(int num = 0) {
        std::barrier bar(num + 1);
        taskqueues = (TaskQueue*)malloc(num * sizeof(TaskQueue));
        if (!taskqueues)
            throw std::bad_alloc();
        threads = (std::thread*)malloc(num * sizeof(std::thread));
        if (!threads)
            throw std::bad_alloc();
        for (int i = 0; i < num; i++) {
            new (&taskqueues[i]) TaskQueue();
            new (&threads[i]) std::thread(thread_worker, this, bar, i);
        }
        bar.arrive_and_wait();
    }
    ~ThreadPool() {}
};
}  // namespace Boundless
#endif  //!_BL_THREADPOOL_HPP_FILE_