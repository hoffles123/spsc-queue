#pragma once

#include <memory>
#include <queue>
#include <atomic>

// lock-based spsc queue
template<typename T, typename Alloc = std::allocator<T>>
class spsc_queue2 {
public:
    using value_type = T;
    using size_type = size_t;
    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits<Alloc>;

private:
    size_type capacity_;
    allocator_type alloc_;
    value_type* ring_;

    std::atomic<size_t> writeIdx_ {};
    std::atomic<size_t> readIdx_ {};

public:

    // non-copyable and non-movable
    spsc_queue2(const spsc_queue2&) = delete;
    spsc_queue2& operator=(const spsc_queue2&) = delete;
    spsc_queue2(spsc_queue2&&) = delete;
    spsc_queue2&& operator=(spsc_queue2&&) = delete;

    explicit spsc_queue2(size_type capacity)
            : capacity_(capacity)
            , ring_(allocator_traits::allocate(alloc_, capacity)) {}

    spsc_queue2(size_type capacity, allocator_type alloc)
            : capacity_(capacity)
            , alloc_(alloc)
            , ring_(allocator_traits::allocate(alloc, capacity)) {}

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

    auto front() noexcept {
        return ring_[readIdx_];
    }

    auto size() noexcept {
        auto diff = writeIdx_ - readIdx_;
        return diff < 0 ? diff + capacity_ : diff;
    }

    auto empty() noexcept {
        return writeIdx_ == readIdx_;
    }

    auto capacity() const noexcept {
        return capacity_ - 1;
    }

};
