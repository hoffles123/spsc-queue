# spsc-queue
SPSC queues are circular buffers that are guaranteed to only have one reader writer pair. 
They are often used as a deterministic message passing mechanism to decouple data producers and consumers.
A common use case is to buffer data streams in real-time systems such as receiving data packets from a network.

## Implementation
Multiple versions of SPSC queues have been implemented (numbered from 1 to 4), each adding optimisations on top of the previous one.

**spscqueue1** <br>
Lock-based spsc queue using `std::mutex` and `std::conditional_variable`. 

**spscqueue2** <br>
Lock-free spsc queue using `std::atomic` and memory barriers.

**spscqueue3** <br>
Identical to spscqueue2, but with added cache alignment and padding to prevent false sharing.

**spscqueue4** <br>
Identical to spscqueue3, but with caching of read and write indexes. This is to reduce L1 cache misses and cache coherency traffic. See [Rigtorp SPSCQueue](https://github.com/rigtorp/SPSCQueue)


## Benchmark
Benchmarked on GCP 2-core
```
------------------------------------------------------------------------------------
Benchmark                          Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------
BM_SPSCQueue<spsc_queue1>        827 ns          810 ns      1000000 ops/sec=1.23517M/s
BM_SPSCQueue<spsc_queue2>       42.8 ns         42.8 ns     16224361 ops/sec=23.3684M/s
BM_SPSCQueue<spsc_queue3>       33.1 ns         33.1 ns     16691355 ops/sec=30.1938M/s
BM_SPSCQueue<spsc_queue4>       6.80 ns         6.80 ns    108407752 ops/sec=147.1M/s
