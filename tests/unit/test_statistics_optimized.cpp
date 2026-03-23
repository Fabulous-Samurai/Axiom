#include <gtest/gtest.h>
#include "statistics_engine.h"

using namespace AXIOM;

TEST(StatisticsEngineTest, MedianOdd) {
    AXIOM::StatisticsEngine engine;
    Vector data = {3.0, 1.0, 2.0};
    auto result = engine.Median(data);
    EXPECT_TRUE(result.HasResult());
    EXPECT_NEAR(*result.GetDouble(), 2.0, 1e-9);
}

TEST(StatisticsEngineTest, MedianEven) {
    AXIOM::StatisticsEngine engine;
    Vector data = {4.0, 1.0, 3.0, 2.0};
    auto result = engine.Median(data);
    EXPECT_TRUE(result.HasResult());
    EXPECT_NEAR(*result.GetDouble(), 2.5, 1e-9);
}

TEST(StatisticsEngineTest, PercentileBasic) {
    AXIOM::StatisticsEngine engine;
    Vector data = {1.0, 2.0, 3.0, 4.0, 5.0};
    // 50th percentile of {1,2,3,4,5} is 3
    auto result = engine.Percentile(data, 50.0);
    EXPECT_TRUE(result.HasResult());
    EXPECT_NEAR(*result.GetDouble(), 3.0, 1e-9);

    // 25th percentile
    // index = 0.25 * (5-1) = 1.0. data[1] = 2.0
    result = engine.Percentile(data, 25.0);
    EXPECT_TRUE(result.HasResult());
    EXPECT_NEAR(*result.GetDouble(), 2.0, 1e-9);
}

TEST(StatisticsEngineTest, PercentileEdge) {
    AXIOM::StatisticsEngine engine;
    Vector data = {10.0, 1.0, 5.0};
    // sorted: 1, 5, 10
    EXPECT_NEAR(*engine.Percentile(data, 0.0).GetDouble(), 1.0, 1e-9);
    EXPECT_NEAR(*engine.Percentile(data, 100.0).GetDouble(), 10.0, 1e-9);
}
