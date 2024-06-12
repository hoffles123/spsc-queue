#include <benchmark/benchmark.h>
#include <thread>
#include "mystl/spscqueue1.h"
#include "mystl/spscqueue2.h"
#include "mystl/spscqueue3.h"
#include "mystl/rigtorp1.h"

void pinThread(int cpu) {
    if (cpu < 0) {
        return;
    }
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
        perror("pthread_setaffinity_no");
        std::exit(EXIT_FAILURE);
    }
}

constexpr auto cpu1 = 1;
constexpr auto cpu2 = 2;

template<template<typename> class SPSCQueueT>
void BM_SPSCQueue(benchmark::State& state) {
    using SPSCQueueType = SPSCQueueT<int>;
    using value_type = typename SPSCQueueType::value_type;

    constexpr size_t queueSize = 100000;

    SPSCQueueType q(queueSize);

    auto t = std::jthread([&] {
        pinThread(cpu1);
        for (auto i = value_type{};; ++i) {
            value_type val;
            while(!q.pop(val))
                ;
            if (val == -1) {
                break;
            }
            if (val != i)
                throw std::runtime_error("runtime error 1");
        }
    });

    auto value = value_type {};
    pinThread(cpu2);
    for (auto _ : state) {
        while(!q.push(value))
            ;
        ++value;
    }
    state.counters["ops/sec"] = benchmark::Counter(double(value), benchmark::Counter::kIsRate);
    state.PauseTiming();
    q.push(-1);
}

//BENCHMARK_TEMPLATE(BM_SPSCQueue, spsc_queue1);
BENCHMARK_TEMPLATE(BM_SPSCQueue, spsc_queue2);
BENCHMARK_TEMPLATE(BM_SPSCQueue, spsc_queue3);
BENCHMARK_TEMPLATE(BM_SPSCQueue, ringbuffer1);

BENCHMARK_MAIN();


