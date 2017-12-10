#include <iostream>
#pragma message("test")
#include <future>
#include <functional>
#include <type_traits>
#include "thread_pool.h"
#include <chrono>
#include <thread>
#include <random>
#include <ctime>
#include <atomic>
using namespace std;
constexpr size_t thr_num = 1000;

struct Test {
    int operator()() {
        cout << "Helo" << endl;
        return 1;
    }
};

int main()
{
    std::mutex cm;
    thread_pool<thr_num> pool;
    atomic_uint64_t count{0};
    alignas(64) atomic_bool start{false}, stop{false};



    cout << "fine" << endl;
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


    thread ps[thr_num];
    for (auto& p : ps)
        p = std::move(thread(work));
    start.store(true, memory_order_release);
    this_thread::sleep_for(1s);
    cout << "SPSC case:" << endl;
    cout << "count: " << count.load(memory_order_acquire) << endl;;
    stop.store(true, memory_order_release);
    for (auto& p : ps)
        p.join();


    //invoke_result_t<Test> a = 5;

    return 0;
}
