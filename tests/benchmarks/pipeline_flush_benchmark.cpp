#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>

static void IfElseBenchmark(benchmark::State& state) {
    std::vector<int> data(1000000, 42);
    data[500000] = 100; // Introduce a maximum value
    for (auto _ : state) {
        int max_val = 0;
        for (int val : data) {
            if (val > max_val) {
                max_val = val;
            }
        }
        benchmark::DoNotOptimize(max_val);
    }
}

static void CmovBenchmark(benchmark::State& state) {
    std::vector<int> data(1000000, 42);
    data[500000] = 100; // Introduce a maximum value
    for (auto _ : state) {
        int max_val = 0;
        for (int val : data) {
            max_val = std::max(max_val, val);
        }
        benchmark::DoNotOptimize(max_val);
    }
}

BENCHMARK(IfElseBenchmark);
BENCHMARK(CmovBenchmark);
BENCHMARK_MAIN();
