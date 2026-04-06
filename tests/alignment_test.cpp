#include <gtest/gtest.h>
#include <cstddef>
#include <iostream>

struct OptimizedOrderBookLevel {
    float price;
    int volume;
};

TEST(AlignmentTest, StructSize) {
    EXPECT_EQ(sizeof(OptimizedOrderBookLevel), 8);
}

TEST(AlignmentTest, MemberOffsets) {
    EXPECT_EQ(offsetof(OptimizedOrderBookLevel, price), 0);
    EXPECT_EQ(offsetof(OptimizedOrderBookLevel, volume), 4);
}

int main() {
    std::cout << "Running Alignment Tests..." << std::endl;
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
