#include <benchmark/benchmark.h>
#include <vector>

// Zero-allocation benchmark example
static void BM_ZeroAllocation(benchmark::State& state) {
    std::vector<int> data(1000, 42); // Pre-allocated
    for (auto _ : state) {
        int sum = 0;
        for (const auto& val : data) {
            sum += val;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_ZeroAllocation);

BENCHMARK_MAIN();
