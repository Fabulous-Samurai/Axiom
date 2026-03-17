#include <gtest/gtest.h>
#include <new>
#include <string>
#include <vector>
#include "optimized_order_book_level.h"
#include "harmonic_string.h"

// Fixture for OptimizedOrderBookLevel tests
class OptimizedOrderBookLevelTest : public ::testing::Test {
protected:
    OptimizedOrderBookLevel level;
};

TEST_F(OptimizedOrderBookLevelTest, SizeIsCorrect) {
    EXPECT_EQ(sizeof(level), 32);
}

TEST_F(OptimizedOrderBookLevelTest, AlignmentIsCorrect) {
    EXPECT_EQ(alignof(OptimizedOrderBookLevel), 32);
}

TEST(HarmonicStringTest, AppendWithinCapacity) {
    HarmonicString str(64);
    str.append("Test");
    EXPECT_EQ(str.size(), 4);
}

TEST(HarmonicStringTest, AppendExceedsCapacity) {
    HarmonicString str(64);
    EXPECT_THROW(str.append(std::string(100, 'A')), std::bad_alloc);
}

TEST(HarmonicStringTest, NoDynamicAllocation) {
    HarmonicString str(64);
    str.append("Test");
    EXPECT_EQ(str.size(), 4);
}

TEST(HardwareAlignmentTest, FalseSharingIsolation) {
    EXPECT_EQ(std::hardware_destructive_interference_size, 64);
}

TEST(HardwareAlignmentTest, ParameterizedTest) {
    for (int i = 1; i <= 50; ++i) {
        HarmonicString str(i * 10);
        EXPECT_NO_THROW(str.append(std::string(i, 'A')));
        EXPECT_EQ(str.size(), i);
    }
}
