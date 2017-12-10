#ifndef COUNT_SEMAPHORE_H
#define COUNT_SEMAPHORE_H
#include <condition_variable>
#include <mutex>
#include <cstddef>

class count_semaphore {
public:
    count_semaphore(std::size_t n);
    count_semaphore(const count_semaphore&) = delete;
    ~count_semaphore() = default;

    void wait();
    void post();
private:
    std::size_t count;
    std::mutex m;
    std::condition_variable cond;
};

#endif 
