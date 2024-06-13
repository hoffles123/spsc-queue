#pragma once

#include <memory>
#include <queue>
#include <atomic>

// basic lock-free spsc queue
template<typename T, typename Alloc = std::allocator<T>>
class spsc_queue2 {
public:
    using value_type = T;
    using size_type = size_t;
    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits<Alloc>;

//private:
    size_type capacity_;
    allocator_type alloc_;
    value_type* ring_;

    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> writeIdx_ {0};
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> readIdx_ {0};

public:

    // non-copyable and non-movable
    spsc_queue2(const spsc_queue2&) = delete;
    spsc_queue2& operator=(const spsc_queue2&) = delete;
    spsc_queue2(spsc_queue2&&) = delete;
    spsc_queue2&& operator=(spsc_queue2&&) = delete;

    explicit spsc_queue2(size_type capacity)
            : capacity_(capacity)
            , ring_(allocator_traits::allocate(alloc_, capacity_)) {}

    spsc_queue2(size_type capacity, allocator_type alloc)
            : capacity_(capacity)
            , alloc_(alloc)
            , ring_(allocator_traits::allocate(alloc, capacity_)) {}

    ~spsc_queue2() {
        while(!empty()) {
            T val {};
            pop(val);
        }
        allocator_traits::deallocate(alloc_, ring_, capacity_);
    }

    auto push(const value_type& val) {
        // writeIdx can be accessed with relaxed memory barrier. only 1 writer
        auto writeIdx = writeIdx_.load(std::memory_order_relaxed);
        auto nextWriteIdx = writeIdx + 1;
        if (nextWriteIdx == capacity_)
            nextWriteIdx = 0;
        if (nextWriteIdx == readIdx_.load(std::memory_order_acquire)) {
            return false;
        }
        new (&ring_[writeIdx]) T(val);
        writeIdx_.store(nextWriteIdx, std::memory_order_release);
        return true;
    }

    bool pop(value_type& val) {
        // readIdx can be accessed with relaxed memory barrier. only 1 reader
        auto readIdx = readIdx_.load(std::memory_order_relaxed);
        if (readIdx == writeIdx_.load(std::memory_order_acquire))
            return false;
        val = ring_[readIdx];
        ring_[readIdx].~T();
        auto nextReadIdx = readIdx + 1;
        if (nextReadIdx == capacity_)
            nextReadIdx = 0;
        readIdx_.store(nextReadIdx, std::memory_order_release);
        return true;
    }

    auto front() const noexcept {
        auto readIdx = readIdx_.load(std::memory_order_relaxed);
        return ring_[readIdx];
    }

    auto size() const noexcept {
        auto readIdx = readIdx_.load(std::memory_order_acquire);
        auto writeIdx = writeIdx_.load(std::memory_order_acquire);
        auto diff = writeIdx - readIdx;
        return diff < 0 ? diff + capacity_ : diff;
    }

    auto empty() const noexcept {
        return size() == 0;
    }

    auto capacity() const noexcept {
        return capacity_ - 1;
    }

};
