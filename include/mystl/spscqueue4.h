#pragma once

#include <memory>
#include <queue>
#include <atomic>
#include <cassert>

// add idx caches
template<typename T, typename Alloc = std::allocator<T>>
class spsc_queue4 {
public:
    using value_type = T;
    using reference = T&;
    using size_type = size_t;
    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits<Alloc>;

    // non-copyable and non-movable
    spsc_queue4(const spsc_queue4&) = delete;
    spsc_queue4& operator=(const spsc_queue4&) = delete;
    spsc_queue4(spsc_queue4&&) = delete;
    spsc_queue4&& operator=(spsc_queue4&&) = delete;

    explicit spsc_queue4(size_type capacity)
            : capacity_(capacity + 1)
            , ring_(allocator_traits::allocate(alloc_, capacity_ + 2 * capPadding)) {}

    spsc_queue4(size_type capacity, allocator_type alloc)
            : capacity_(capacity + 1)
            , alloc_(alloc)
            , ring_(allocator_traits::allocate(alloc, capacity_ + 2 * capPadding)) {}

    ~spsc_queue4() {
        value_type val;
        while(pop(val))
            ;
        allocator_traits::deallocate(alloc_, ring_, capacity_ + 2 * capPadding);
    }

    void push(const value_type& val) {
        auto writeIdx = writeIdx_.load(std::memory_order_relaxed);
        auto nextWriteIdx = writeIdx + 1;
        if (nextWriteIdx == capacity_)
            nextWriteIdx = 0;
        while(nextWriteIdx == readIdxCache_)
            readIdxCache_ = readIdx_.load(std::memory_order_acquire); // spin lock
        new (&ring_[writeIdx + capPadding]) T(val);
        writeIdx_.store(nextWriteIdx, std::memory_order_release);
    }

    bool pop(reference val) {
        auto readIdx = readIdx_.load(std::memory_order_relaxed);
        if (readIdx == writeIdxCache_) {
            writeIdxCache_ = writeIdx_.load(std::memory_order_acquire);
            if (readIdx == writeIdxCache_)
                return false;
        }
        val = ring_[readIdx + capPadding];
        ring_[readIdx + capPadding].~T();
        auto nextReadIdx = readIdx + 1;
        if (nextReadIdx == capacity_)
            nextReadIdx = 0;
        readIdx_.store(nextReadIdx, std::memory_order_release);
        return true;
    }

    reference front() const noexcept {
        auto readIdx = readIdx_.load(std::memory_order_relaxed);
        return ring_[readIdx + capPadding];
    }

    size_type size() const noexcept {
        auto readIdx = readIdx_.load(std::memory_order_acquire);
        auto writeIdx = writeIdx_.load(std::memory_order_acquire);
        auto diff = writeIdx - readIdx;
        return diff < 0 ? diff + capacity_ : diff;
    }

    bool empty() const noexcept {
        return writeIdx_.load(std::memory_order_acquire) ==
               readIdx_.load(std::memory_order_acquire);
    }

    size_type capacity() const noexcept {
        return capacity_ - 1;
    }

private:
    size_type capacity_;
    allocator_type alloc_;
    value_type* ring_;

#ifdef __cpp_lib_hardware_interference_size
    static constexpr auto cacheLineSize = std::hardware_destructive_interference_size;
#else
    static constexpr auto cacheLineSize = 64;
#endif

    // num of elements that at least fit into a cache line
    static constexpr auto capPadding = (cacheLineSize - 1) / sizeof(value_type) + 1;

    alignas(cacheLineSize) std::atomic<size_t> writeIdx_ {0};
    alignas(cacheLineSize) std::atomic<size_t> readIdx_ {0};
    alignas(cacheLineSize) size_t writeIdxCache_ {0};
    alignas(cacheLineSize) size_t readIdxCache_ {0};
};
