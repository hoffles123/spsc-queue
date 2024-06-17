#pragma once

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

// lock-based spsc queue
template<typename T, typename Alloc = std::allocator<T>>
class spsc_queue1 {
public:
    using value_type = T;
    using reference = T&;
    using size_type = size_t;
    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits<Alloc>;

    // non-copyable and non-movable
    spsc_queue1(const spsc_queue1&) = delete;
    spsc_queue1& operator=(const spsc_queue1&) = delete;
    spsc_queue1(spsc_queue1&&) = delete;
    spsc_queue1&& operator=(spsc_queue1&&) = delete;

    explicit spsc_queue1(size_type capacity)
        : capacity_(capacity + 1)
        , ring_(allocator_traits::allocate(alloc_, capacity + 1)) {}

    spsc_queue1(size_type capacity, allocator_type alloc)
        : capacity_(capacity + 1)
        , alloc_(alloc)
        , ring_(allocator_traits::allocate(alloc, capacity + 1)) {}

    ~spsc_queue1() {
        while(!empty()) {
            pop();
        }
        allocator_traits::deallocate(alloc_, ring_, capacity_);
    }

    void push(const value_type& val) {
        auto nextWriteIdx = getNextWriteIdx();
        std::unique_lock lock(mut);
        cv.wait(lock, [nextWriteIdx, this] {
            return nextWriteIdx != readIdx_;
        });
        new (&ring_[writeIdx_]) T(val);
        writeIdx_ = nextWriteIdx;
    }

    void pop() {
        std::scoped_lock lock(mut);
        assert(writeIdx_ != readIdx_ && "queue cannot be empty when popping");
        auto nextWriteIdx = getNextWriteIdx();
        bool wasFull = nextWriteIdx == readIdx_;
        ring_[readIdx_].~T();
        ++readIdx_;
        if (readIdx_ == capacity_)
            readIdx_ = 0;
        if (wasFull)
            cv.notify_one();
    }

    reference front() noexcept {
        std::scoped_lock lock(mut);
        return ring_[readIdx_];
    }

    size_type size() noexcept {
        std::scoped_lock lock(mut);
        auto diff = writeIdx_ - readIdx_;
        return diff < 0 ? diff + capacity_ : diff;
    }

    bool empty() noexcept {
        std::scoped_lock lock(mut);
        return writeIdx_ == readIdx_;
    }

    size_type capacity() const noexcept {
        return capacity_ - 1;
    }


private:
    size_type getNextWriteIdx() {
        auto nextWriteIdx = writeIdx_ + 1;
        if (nextWriteIdx == capacity_)
            nextWriteIdx = 0;
        return nextWriteIdx;
    }

    size_type capacity_;
    allocator_type alloc_;
    value_type* ring_;

    std::mutex mut;
    std::condition_variable cv;

    size_type writeIdx_ {0};
    size_type readIdx_ {0};
};
