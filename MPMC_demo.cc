#include <iostream>
#include <future>
#include <functional>
#include "thread_pool.h"
#include <chrono>
#include <thread>
#include <random>
#include <ctime>
#include <atomic>
using namespace std;
constexpr size_t thr_num = 5;


int main()
{
    thread_pool<thr_num> pool;
    atomic_uint64_t count{0};
    alignas(64) atomic_bool start{false}, stop{false};

    auto random_sum = [&] {
        mt19937 engine(time(0));
        uniform_int_distribution<unsigned> random_generate(0, 10000000);
        unsigned num = random_generate(engine);
        unsigned total = 0;
        for (unsigned i = 0; i != 100; ++i, ++num)
            total += num;
        count.fetch_add(1, memory_order_release);
        return total;
    };


    auto work = [&] {
        while (!start.load(memory_order_acquire))
            continue;
        while (!stop.load(memory_order_acquire))
            pool.add_task(random_sum);
    };


    constexpr size_t prod_size = thr_num / 2;
    thread ps[prod_size];
    for (auto& p : ps)
        p = std::move(thread(work));
    start.store(true, memory_order_release);
    this_thread::sleep_for(10s);
    cout << "MPMC case:" << endl;
    cout << "count: " << count.load(memory_order_acquire) / 10 << endl;;
    stop.store(true, memory_order_release);
    for (auto& p : ps)
        p.join();

    return 0;
}
