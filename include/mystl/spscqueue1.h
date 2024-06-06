#pragma once

#include <memory>
#include <queue>

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

    size_type writeIdx_ {};
    size_type readIdx_ {};

    std::mutex mut;
    std::condition_variable cv;

public:

    // non-copyable and non-movable
    spsc_queue1(const spsc_queue1&) = delete;
    spsc_queue1& operator=(const spsc_queue1&) = delete;
    spsc_queue1(spsc_queue1&&) = delete;
    spsc_queue1&& operator=(spsc_queue1&&) = delete;

    explicit spsc_queue1(size_type capacity)
        : capacity_(capacity)
        , ring_(allocator_traits::allocate(alloc_, capacity)) {
        // one unused slot to disambiguate empty/full
        capacity_++;
    }

    spsc_queue1(size_type capacity, allocator_type alloc)
        : capacity_(capacity)
        , alloc_(alloc)
        , ring_(allocator_traits::allocate(alloc, capacity)) {}

    ~spsc_queue1() {
        while(!empty()) {
            ring_[readIdx_ % capacity_].~T();
            ++readIdx_;
        }
        allocator_traits::deallocate(alloc_, ring_, capacity_);
    }

    auto push(const value_type& val) {
        std::scoped_lock lock(mut);
        if (writeIdx_ + 1 == readIdx_) {
            return false;
        }
        const bool wasEmpty = writeIdx_ == readIdx_;
        new (ring_[writeIdx_]) T(val);
        ++writeIdx_;
        if (writeIdx_ == capacity_)
            writeIdx_ = 0;
        // only when writeIdx == readIdx, will there be contention
        if (wasEmpty)
            cv.notify_one();
        return true;
    }

    // exception safety: accept a reference instead of return by value
    // if copy constructor of by-return value throws, value is lost as it has been removed from queue
    void pop(value_type& val) {
        std::unique_lock lock(mut);
        // if empty, wait for write
        while(writeIdx_ == readIdx_)
            cv.wait(lock);
        val = ring_[readIdx_];
        ring_[readIdx_].~T();
        ++readIdx_;
        if (readIdx_ == capacity_)
            readIdx_ = 0;
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
