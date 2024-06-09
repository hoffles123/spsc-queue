#include <gtest/gtest.h>
#include <thread>
#include "mystl/spscqueue1.h"

template<typename SPSCQueueT>
class TestBase : public testing::Test {
public:
    using SPSCQueueType = SPSCQueueT;

    SPSCQueueT queue {2};
    SPSCQueueT queue2 {1000};
};

template<typename SPSCQueueT>
using SPSCQueueTests = TestBase<SPSCQueueT>;

using SPSCQueueTypes = ::testing::Types<
        spsc_queue1<int>
        >;

TYPED_TEST_SUITE(SPSCQueueTests, SPSCQueueTypes);

TYPED_TEST(SPSCQueueTests, TestProperties) {
    EXPECT_FALSE(std::is_copy_constructible_v<typename TestFixture::SPSCQueueType>);
    EXPECT_FALSE(std::is_move_constructible_v<typename TestFixture::SPSCQueueType>);
    EXPECT_FALSE(std::is_copy_assignable_v<typename TestFixture::SPSCQueueType>);
    EXPECT_FALSE(std::is_move_assignable_v<typename TestFixture::SPSCQueueType>);
}

TYPED_TEST(SPSCQueueTests, TestInitConditions) {
    EXPECT_EQ(this->queue.capacity(), 2);
    EXPECT_EQ(this->queue.size(), 0);
    EXPECT_TRUE(this->queue.empty());
}

TYPED_TEST(SPSCQueueTests, TestPush) {
    EXPECT_TRUE(this->queue.push(1));
    EXPECT_EQ(this->queue.size(), 1);
    EXPECT_FALSE(this->queue.empty());

    EXPECT_TRUE(this->queue.push(2));
    EXPECT_EQ(this->queue.size(), 2);

    EXPECT_FALSE(this->queue.push(3));
}

TYPED_TEST(SPSCQueueTests, TestPop) {
    EXPECT_TRUE(this->queue.push(1));
    EXPECT_TRUE(this->queue.push(2));

    int front {};
    EXPECT_TRUE(this->queue.pop(front));
    EXPECT_EQ(front, 1);
    EXPECT_TRUE(this->queue.pop(front));
    EXPECT_EQ(front, 2);
    EXPECT_TRUE(this->queue.empty());
    EXPECT_EQ(this->queue.size(), 0);
    EXPECT_FALSE(this->queue.pop(front));
}

constexpr auto cpu1 = 1;
constexpr auto cpu2 = 2;

void pinThread(int cpu) {
    if (cpu < 0) {
        return;
    }
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
        perror("pthread_setaffinity_no");
        std::exit(EXIT_FAILURE);
    }
}

TYPED_TEST(SPSCQueueTests, TestThreads) {
    constexpr int64_t iters = 10000000;

    auto t = std::jthread([&] {
        pinThread(cpu1);
        for (int64_t i {0}; i < iters; ++i) {
            int val;
            while(!this->queue2.pop(val))
                ;
            if (val != i)
                throw std::runtime_error("runtime error 1");

        }
    });

    pinThread(cpu2);

    for (int64_t i {0}; i < iters; ++i) {
        while (!this->queue2.push(i))
            ;
    }
}