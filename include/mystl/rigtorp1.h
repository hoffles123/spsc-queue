#include <vector>
#include <atomic>

template<typename T>
struct ringbuffer1 {
    std::vector<int> data_{};
    std::atomic<size_t> readIdx_{0};
    std::atomic<size_t> writeIdx_{0};

    using value_type = T;

    ringbuffer1(size_t capacity) : data_(capacity, 0) {}

    bool push(int val) {
        auto const writeIdx = writeIdx_.load(std::memory_order_relaxed);
        auto nextWriteIdx = writeIdx + 1;
        if (nextWriteIdx == data_.size()) {
            nextWriteIdx = 0;
        }
        if (nextWriteIdx == readIdx_.load(std::memory_order_acquire)) {
            return false;
        }
        data_[writeIdx] = val;
        writeIdx_.store(nextWriteIdx, std::memory_order_release);
        return true;
    }

    bool pop(int &val) {
        auto const readIdx = readIdx_.load(std::memory_order_relaxed);
        if (readIdx == writeIdx_.load(std::memory_order_acquire)) {
            return false;
        }
        val = data_[readIdx];
        auto nextReadIdx = readIdx + 1;
        if (nextReadIdx == data_.size()) {
            nextReadIdx = 0;
        }
        readIdx_.store(nextReadIdx, std::memory_order_release);
        return true;
    }
};
