#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <list>
#include <mutex>
#include <thread>
#include <csignal>
#include <tuple>
#include <functional>
#include <memory>
#include <atomic>
#include "count_semaphore.h"
#include <iostream>


template <std::size_t thread_num=4>
class thread_pool {
public:
    thread_pool();
    thread_pool(const thread_pool&) = delete;
    ~thread_pool();

    template <typename Func, typename... Args>
    std::future<std::invoke_result_t<Func, Args...>> add_task(Func&&, Args&&... args);
    template <typename Func>
    std::future<std::invoke_result_t<Func>> add_task(Func&&);

    std::function<void(int)> get_signal_handler();

private:
    std::array<std::thread, thread_num> workers;

    count_semaphore has_job;
    std::atomic_size_t idle_threads;
    std::list<std::function<void()>> queue;
    std::mutex q_mutex;
    std::sig_atomic_t quit;
};

template <std::size_t N>
thread_pool<N>::thread_pool()
    : has_job(0), idle_threads(N), quit(0)
{
    using namespace std;
    auto routine = [this] 
    {

        while (!quit) {
            this->has_job.wait();
            if (quit)
                break;
            idle_threads.fetch_sub(1, std::memory_order_release);
            std::unique_lock<std::mutex> lock(this->q_mutex);
            auto job = this->queue.front();
            this->queue.pop_front();
            lock.unlock();
            
            job();
            idle_threads.fetch_add(1, std::memory_order_release);
        }
    };


    for (auto& worker: workers) {
        worker = std::thread(routine);
    }
}

template <std::size_t N>
thread_pool<N>::~thread_pool()
{
    using namespace std;
    quit = 1;

    for (int i = 0; i != idle_threads.load(); ++i)
        has_job.post();
    for (auto& worker : workers) 
        worker.join();
}

template <std::size_t N>
template <typename Func, typename... Args>
std::future<std::invoke_result_t<Func, Args...>> thread_pool<N>::add_task(Func&& func, Args&&... args)
{
    typedef std::invoke_result_t<Func, Args...> result_type;
    std::packaged_task<std::function<result_type(Args...)>> task = [func, args...] { return func(args...); };
    {
        std::lock_guard<std::mutex> lock(q_mutex);
        queue.emplace_back([task] { task(); });
    }
    has_job.post();

    return task.get_future();
}

template <std::size_t N>
template <typename Func>
std::future<std::invoke_result_t<Func>> thread_pool<N>::add_task(Func&& func)
{
    using namespace std;
    typedef std::invoke_result_t<Func> result_type;
    auto task = std::make_shared<std::packaged_task<result_type()>>([func] { return func(); });
    std::unique_lock<std::mutex> lock(q_mutex);
    queue.emplace_back([task] { (*task)(); });
    lock.unlock();
    has_job.post();

    return task->get_future();
}

template <std::size_t N>
std::function<void(int)> thread_pool<N>::get_signal_handler()
{
    return [this] (int) { this->quit = 1; };
}

#endif 
