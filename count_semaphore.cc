#include "count_semaphore.h"
#include <condition_variable>
#include <mutex>
#include <cstddef>

count_semaphore::count_semaphore(std::size_t n)
    : count(n)
{}

void count_semaphore::wait()
{
    std::unique_lock<std::mutex> lock(m);
    cond.wait(lock, [this] { return count > 0; });
    --count;
}

void count_semaphore::post()
{
    std::lock_guard<std::mutex> lock(m);
    ++count;
    cond.notify_one();
}

