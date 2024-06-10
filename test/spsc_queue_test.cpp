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

TYPED_TEST(SPSCQueueTests, TestConcurrentPushPop) {
    constexpr int64_t iters = 100000;

    auto t = std::jthread([&] {
        for (int64_t i {0}; i < iters; ++i) {
            int val;
            while(!this->queue2.pop(val))
                ;
            EXPECT_EQ(val, i);
        }
    });

    for (int64_t i {0}; i < iters; ++i) {
        while (!this->queue2.push(i))
            ;
    }
}