#include <gtest/gtest.h>
#include <thread>
#include "mystl/spscqueue1.h"
#include "mystl/spscqueue2.h"
#include "mystl/spscqueue3.h"

template<typename SPSCQueueT>
class TestBase : public testing::Test {
public:
    using SPSCQueueType = SPSCQueueT;

    SPSCQueueT queue {2};
    SPSCQueueT queue2 {10};
};

template<typename SPSCQueueT>
using SPSCQueueTests = TestBase<SPSCQueueT>;

using SPSCQueueTypes = ::testing::Types<
        spsc_queue1<int>,
        spsc_queue2<int>,
        spsc_queue3<int>
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
    this->queue.push(1);
    EXPECT_EQ(this->queue.size(), 1);
    EXPECT_FALSE(this->queue.empty());

    this->queue.push(2);
    EXPECT_EQ(this->queue.size(), 2);
    EXPECT_EQ(this->queue.front(), 1);
}

TYPED_TEST(SPSCQueueTests, TestPushPop) {
    this->queue.push(1);
    this->queue.push(2);

    EXPECT_EQ(this->queue.front(), 1);
    this->queue.pop();
    EXPECT_EQ(this->queue.front(), 2);
    this->queue.pop();
    EXPECT_TRUE(this->queue.empty());
    EXPECT_EQ(this->queue.size(), 0);

    this->queue.push(3);
    EXPECT_EQ(this->queue.front(), 3);
    this->queue.pop();
    EXPECT_TRUE(this->queue.empty());
    EXPECT_EQ(this->queue.size(), 0);
}

TYPED_TEST(SPSCQueueTests, TestConcurrentPushPop) {
    constexpr int iters = 100000;

    auto t = std::jthread([&] {
        for (int i {0}; i < iters; ++i) {
            while(this->queue2.empty())
                ;
            int front = this->queue2.front();
            EXPECT_EQ(front, i);
            this->queue2.pop();
        }
        EXPECT_TRUE(this->queue2.empty());
    });

    for (int i {0}; i < iters; ++i) {
        this->queue2.push(i);
    }
}