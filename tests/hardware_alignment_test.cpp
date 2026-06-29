#include <gtest/gtest.h>
#include <new>
#include <string>
#include "optimized_order_book_level.h"
#include "harmonic_string.h"

TEST(HardwareAlignmentTest, OptimizedOrderBookLevelSize) {
    EXPECT_EQ(sizeof(OptimizedOrderBookLevel), 32);
}

TEST(HardwareAlignmentTest, FalseSharingIsolation) {
    EXPECT_EQ(std::hardware_destructive_interference_size, 64);
}

TEST(HarmonicStringTest, NoDynamicAllocation) {
    HarmonicString str(64);
    str.append("Test");
    EXPECT_EQ(str.size(), 4);
    EXPECT_THROW(str.append(std::string(100, 'A')), std::bad_alloc);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
