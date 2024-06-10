#pragma once

#include <memory>
#include <queue>
#include <mutex>

// lock-based spsc queue
template<typename T, typename Alloc = std::allocator<T>>
class spsc_queue1 {
public:
    using value_type = T;
    using size_type = size_t;
    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits<Alloc>;

private:
    size_type capacity_;
    allocator_type alloc_;
    value_type* ring_;
    std::mutex mut;

    size_type writeIdx_ {};
    size_type readIdx_ {};

public:

    // non-copyable and non-movable
    spsc_queue1(const spsc_queue1&) = delete;
    spsc_queue1& operator=(const spsc_queue1&) = delete;
    spsc_queue1(spsc_queue1&&) = delete;
    spsc_queue1&& operator=(spsc_queue1&&) = delete;

    explicit spsc_queue1(size_type capacity)
        : capacity_(capacity)
        , ring_(allocator_traits::allocate(alloc_, capacity)) {}

    spsc_queue1(size_type capacity, allocator_type alloc)
        : capacity_(capacity)
        , alloc_(alloc)
        , ring_(allocator_traits::allocate(alloc, capacity)) {}

    ~spsc_queue1() {
        while(!empty()) {
            T val {};
            pop(val);
        }
        allocator_traits::deallocate(alloc_, ring_, capacity_);
    }

    auto push(const value_type& val) {
        auto nextWriteIdx = writeIdx_ + 1;
        if (nextWriteIdx == capacity_)
            nextWriteIdx = 0;
        std::scoped_lock lock(mut);
        if (nextWriteIdx == readIdx_) {
            return false;
        }
        new (&ring_[writeIdx_]) T(val);
        writeIdx_ = nextWriteIdx;
        return true;
    }

    // exception safety: accept a reference instead of return by value
    // if copy constructor of by-return value throws,
    // value is lost as it has been removed from queue
    bool pop(value_type& val) {
        std::scoped_lock lock(mut);
        if (readIdx_ == writeIdx_)
            return false;
        val = ring_[readIdx_];
        ring_[readIdx_].~T();
        ++readIdx_;
        if (readIdx_ == capacity_)
            readIdx_ = 0;
        return true;
    }

    auto front() noexcept {
        std::scoped_lock lock(mut);
        return ring_[readIdx_];
    }

    auto size() noexcept {
        std::scoped_lock lock(mut);
        auto diff = writeIdx_ - readIdx_;
        return diff < 0 ? diff + capacity_ : diff;
    }

    auto empty() noexcept {
        std::scoped_lock lock(mut);
        return writeIdx_ == readIdx_;
    }

    auto capacity() const noexcept {
        return capacity_ - 1;
    }

};
