# cxx17-ThreadPool
This implementation is inspired by [progschj/ThreadPool] and [Pithikos/C-Thread-Pool].

A simple thread pool written in C++17.

# How to use it

just include all the sources into your project and compile:

`g++ main.cc count_semaphore.cc -std=c++17 -pthread -latomic -o example.out`

then you are done.

[progschj/ThreadPool]: https://github.com/progschj/ThreadPool
[Pithikos/C-Thread-Pool]: https://github.com/Pithikos/C-Thread-Pool
