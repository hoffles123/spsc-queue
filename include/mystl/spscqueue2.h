#pragma once

#include <memory>
#include <queue>
#include <atomic>

// basic lock-free spsc queue
template<typename T, typename Alloc = std::allocator<T>>
class spsc_queue2 {
public:
    using value_type = T;
    using reference = T&;
    using size_type = size_t;
    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits<Alloc>;

    // non-copyable and non-movable
    spsc_queue2(const spsc_queue2&) = delete;
    spsc_queue2& operator=(const spsc_queue2&) = delete;
    spsc_queue2(spsc_queue2&&) = delete;
    spsc_queue2&& operator=(spsc_queue2&&) = delete;

    explicit spsc_queue2(size_type capacity)
            : capacity_(capacity + 1)
            , ring_(allocator_traits::allocate(alloc_, capacity + 1)) {}

    spsc_queue2(size_type capacity, allocator_type alloc)
            : capacity_(capacity + 1)
            , alloc_(alloc)
            , ring_(allocator_traits::allocate(alloc, capacity + 1)) {}

    ~spsc_queue2() {
        while(!empty()) {
            pop();
        }
        allocator_traits::deallocate(alloc_, ring_, capacity_);
    }

    void push(const value_type& val) {
        auto writeIdx = writeIdx_.load(std::memory_order_relaxed);
        auto nextWriteIdx = writeIdx + 1;
        if (nextWriteIdx == capacity_)
            nextWriteIdx = 0;
        while(nextWriteIdx == readIdx_.load(std::memory_order_acquire))
            ; // spin lock
        new (&ring_[writeIdx]) T(val);
        writeIdx_.store(nextWriteIdx, std::memory_order_release);
    }

    void pop() {
        auto readIdx = readIdx_.load(std::memory_order_relaxed);
        assert(readIdx != writeIdx_.load(std::memory_order_acquire) &&
            "queue cannot be empty when popping");
        ring_[readIdx].~T();
        auto nextReadIdx = readIdx + 1;
        if (nextReadIdx == capacity_)
            nextReadIdx = 0;
        readIdx_.store(nextReadIdx, std::memory_order_release);
    }

    reference front() const noexcept {
        auto readIdx = readIdx_.load(std::memory_order_relaxed);
        return ring_[readIdx];
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

    std::atomic<size_t> writeIdx_ {0};
    std::atomic<size_t> readIdx_ {0};
};
