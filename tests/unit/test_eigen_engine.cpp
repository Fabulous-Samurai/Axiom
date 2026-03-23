#include <gtest/gtest.h>
#include "eigen_engine.h"
#include <vector>

#ifdef ENABLE_EIGEN

using namespace AXIOM;

TEST(EigenEngineTest, Initialization) {
    EigenEngine engine;
    engine.SetOptimizationLevel(CPUOptimizationLevel::Basic);
    engine.SetNumThreads(1);
    // Basic verification that it doesn't crash and reports correctly
    auto report = engine.GetPerformanceReport();
    EXPECT_FALSE(report.empty());
}

TEST(EigenEngineTest, MatrixVectorCreation) {
    EigenEngine engine;
    std::vector<std::vector<double>> mat_data = {{1.0, 2.0}, {3.0, 4.0}};
    auto mat = engine.CreateMatrix(mat_data);
    EXPECT_EQ(mat.rows(), 2);
    EXPECT_EQ(mat.cols(), 2);
    EXPECT_DOUBLE_EQ(mat(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(mat(1, 1), 4.0);

    std::vector<double> vec_data = {1.0, 2.0, 3.0};
    auto vec = engine.CreateVector(vec_data);
    EXPECT_EQ(vec.size(), 3);
    EXPECT_DOUBLE_EQ(vec(0), 1.0);
    EXPECT_DOUBLE_EQ(vec(2), 3.0);
}

TEST(EigenEngineTest, BasicArithmetic) {
    EigenEngine engine;
    auto A = engine.CreateMatrix({{1.0, 2.0}, {3.0, 4.0}});
    auto B = engine.CreateMatrix({{5.0, 6.0}, {7.0, 8.0}});

    auto sum = engine.MatrixAdd(A, B);
    EXPECT_DOUBLE_EQ(sum(0, 0), 6.0);
    EXPECT_DOUBLE_EQ(sum(1, 1), 12.0);

    auto diff = engine.MatrixSubtract(B, A);
    EXPECT_DOUBLE_EQ(diff(0, 0), 4.0);
    EXPECT_DOUBLE_EQ(diff(1, 1), 4.0);

    auto product = engine.MatrixMultiply(A, B);
    // [1 2] [5 6]   [1*5+2*7 1*6+2*8]   [19 22]
    // [3 4] [7 8] = [3*5+4*7 3*6+4*8] = [43 50]
    EXPECT_DOUBLE_EQ(product(0, 0), 19.0);
    EXPECT_DOUBLE_EQ(product(1, 1), 50.0);
}

TEST(EigenEngineTest, AdvancedOperations) {
    EigenEngine engine;
    auto A = engine.CreateMatrix({{1.0, 2.0}, {3.0, 4.0}});

    EXPECT_DOUBLE_EQ(engine.Determinant(A), -2.0);

    auto inv_opt = engine.Inverse(A);
    ASSERT_TRUE(inv_opt.has_value());
    auto identity = engine.MatrixMultiply(A, *inv_opt);
    EXPECT_NEAR(identity(0, 0), 1.0, 1e-9);
    EXPECT_NEAR(identity(1, 1), 1.0, 1e-9);
    EXPECT_NEAR(identity(0, 1), 0.0, 1e-9);

    auto AT = engine.Transpose(A);
    EXPECT_DOUBLE_EQ(AT(0, 1), 3.0);
    EXPECT_DOUBLE_EQ(AT(1, 0), 2.0);
}

TEST(EigenEngineTest, LinearSystem) {
    EigenEngine engine;
    // 2x + y = 8
    // x + 3y = 13
    // -> x = 2.2, y = 3.6
    auto A = engine.CreateMatrix({{2.0, 1.0}, {1.0, 3.0}});
    auto b = engine.CreateVector({8.0, 13.0});
    auto x = engine.SolveLinearSystem(A, b);
    EXPECT_NEAR(x(0), 2.2, 1e-7);
    EXPECT_NEAR(x(1), 3.6, 1e-7);
}

TEST(EigenEngineTest, SignalProcessing) {
    EigenEngine engine;
    // Simple impulse
    auto impulse = engine.CreateVector({1.0, 0.0, 0.0, 0.0});
    auto fft = engine.FFT(impulse);
    // FFT of [1, 0, 0, 0] should be [1, 1, 1, 1] (magnitudes)
    for(int i=0; i<4; ++i) {
        EXPECT_NEAR(fft(i), 1.0, 1e-7);
    }

    auto restored = engine.IFFT(fft);
    // If I pass [1, 1, 1, 1] to IFFT:
    // Freq: [1+0i, 1+0i, 1+0i, 1+0i]
    // IFFT: [1, 0, 0, 0]
    EXPECT_NEAR(restored(0), 1.0, 1e-7);
    EXPECT_NEAR(restored(1), 0.0, 1e-7);

    // Convolution
    auto s1 = engine.CreateVector({1.0, 2.0});
    auto s2 = engine.CreateVector({3.0, 4.0});
    // [1, 2] * [3, 4] = [3, 4+6, 8] = [3, 10, 8]
    auto conv = engine.Convolution(s1, s2);
    ASSERT_EQ(conv.size(), 3);
    EXPECT_NEAR(conv(0), 3.0, 1e-7);
    EXPECT_NEAR(conv(1), 10.0, 1e-7);
    EXPECT_NEAR(conv(2), 8.0, 1e-7);
}

TEST(EigenEngineTest, Statistics) {
    EigenEngine engine;
    auto data = engine.CreateVector({1.0, 2.0, 3.0, 4.0, 5.0});
    EXPECT_DOUBLE_EQ(engine.Mean(data), 3.0);
    EXPECT_NEAR(engine.StandardDeviation(data), 1.41421356, 1e-7);

    auto normalized = engine.Normalize(data);
    EXPECT_NEAR(engine.Mean(normalized), 0.0, 1e-7);
    EXPECT_NEAR(engine.StandardDeviation(normalized), 1.0, 1e-7);
}

#else
TEST(EigenEngineTest, Disabled) {
    SUCCEED() << "EigenEngine tests skipped because ENABLE_EIGEN is not defined";
}
#endif
