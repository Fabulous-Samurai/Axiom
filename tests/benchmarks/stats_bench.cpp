#include "../../include/statistics_engine.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>

using namespace AXIOM;

static std::vector<double> generate_random_vector(size_t size) {
    std::vector<double> data(size);
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(0.0, 1000.0);
    for (size_t i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }
    return data;
}

static void BM_Median(benchmark::State& state) {
    size_t size = state.range(0);
    StatisticsEngine engine;
    auto data = generate_random_vector(size);
    for (auto _ : state) {
        // Measure the whole thing including the pass-by-value/copy
        auto res = engine.Median(data);
        benchmark::DoNotOptimize(res);
    }
}
BENCHMARK(BM_Median)->RangeMultiplier(10)->Range(100, 100000);

static void BM_Percentile(benchmark::State& state) {
    size_t size = state.range(0);
    StatisticsEngine engine;
    auto data = generate_random_vector(size);
    for (auto _ : state) {
        auto res = engine.Percentile(data, 50.0);
        benchmark::DoNotOptimize(res);
    }
}
BENCHMARK(BM_Percentile)->RangeMultiplier(10)->Range(100, 100000);

BENCHMARK_MAIN();
