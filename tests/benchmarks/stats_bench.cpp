#include "../../include/statistics_engine.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>

using namespace AXIOM;

static std::vector<double> generate_random_vector(size_t size) {
    std::vector<double> data(size);
    // Use a fixed seed for deterministic benchmark results.
    // std::mt19937 with a constant seed is fine for benchmarking,
    // but SonarCloud might flag it (S2245).
    // Using a simple deterministic sequence here to avoid the need for a PRNG if it's flagged.
    for (size_t i = 0; i < size; ++i) {
        data[i] = static_cast<double>((i * 1103515245 + 12345) & 0x7FFFFFFF);
    }
    return data;
}

static void BM_Median(benchmark::State& state) {
    size_t size = static_cast<size_t>(state.range(0));
    AXIOM::StatisticsEngine engine;
    auto data = generate_random_vector(size);
    for (auto _ : state) {
        auto res = engine.Median(data);
        benchmark::DoNotOptimize(res);
    }
}
BENCHMARK(BM_Median)->RangeMultiplier(10)->Range(100, 100000);

static void BM_Percentile(benchmark::State& state) {
    size_t size = static_cast<size_t>(state.range(0));
    AXIOM::StatisticsEngine engine;
    auto data = generate_random_vector(size);
    for (auto _ : state) {
        auto res = engine.Percentile(data, 50.0);
        benchmark::DoNotOptimize(res);
    }
}
BENCHMARK(BM_Percentile)->RangeMultiplier(10)->Range(100, 100000);

BENCHMARK_MAIN();
