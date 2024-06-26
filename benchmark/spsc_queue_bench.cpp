#include <benchmark/benchmark.h>
#include <thread>
#include <iostream>

#include "pinToCPU.h"
#include "mystl/spscqueue1.h"
#include "mystl/spscqueue2.h"
#include "mystl/spscqueue3.h"
#include "mystl/spscqueue4.h"

constexpr auto cpu1 = 0;
constexpr auto cpu2 = 3;

template<template<typename> class SPSCQueueT>
void BM_SPSCQueue(benchmark::State& state) {
    using SPSCQueueType = SPSCQueueT<int>;

    constexpr size_t queueSize = 100000;

    SPSCQueueType q(queueSize);

    auto consumer = std::jthread([&] {
        pinThread(cpu1);
        for (int i {0};; ++ i) {
            int val {};
            while(!q.pop(val))
                ;
            if (val == -1)
                break;
            if (val != i)
                throw std::runtime_error("wrong value in queue");
        }
    });

    int count {0};
    pinThread(cpu2);
    for (auto _ : state) {
        q.push(count);
        count++;
    }

    state.counters["ops/sec"] = benchmark::Counter(static_cast<double>(count),
                                                   benchmark::Counter::kIsRate);
    state.PauseTiming();
    q.push(-1);
}

BENCHMARK_TEMPLATE(BM_SPSCQueue, spsc_queue1);
BENCHMARK_TEMPLATE(BM_SPSCQueue, spsc_queue2);
BENCHMARK_TEMPLATE(BM_SPSCQueue, spsc_queue3);
BENCHMARK_TEMPLATE(BM_SPSCQueue, spsc_queue4);

BENCHMARK_MAIN();


