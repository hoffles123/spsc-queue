#include <gtest/gtest.h>
#include "mystl/spscqueue1.h"

template<typename SPSCQueueT>
class TestBase : public testing::Test {
public:
    using SPSCQueueType = SPSCQueueT;

    SPSCQueueT queue {4};
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
    EXPECT_EQ(this->queue.capacity(), 4);
    EXPECT_EQ(this->queue.size(), 0);
    EXPECT_TRUE(this->queue.empty());
}

TYPED_TEST(SPSCQueueTests, TestPush) {

}