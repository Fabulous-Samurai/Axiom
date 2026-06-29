#include <benchmark/benchmark.h>
#include <vector>
#include <numeric>

struct AoS {
    float price;
    int volume;
};

static void AoSBenchmark(benchmark::State& state) {
    std::vector<AoS> data(10000000, {42.0f, 100});
    for (auto _ : state) {
        float sum = 0;
        for (const auto& entry : data) {
            sum += entry.price;
        }
        benchmark::DoNotOptimize(sum);
    }
}

static void SoABenchmark(benchmark::State& state) {
    std::vector<float> prices(10000000, 42.0f);
    std::vector<int> volumes(10000000, 100);
    for (auto _ : state) {
        float sum = std::accumulate(prices.begin(), prices.end(), 0.0f);
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK(AoSBenchmark);
BENCHMARK(SoABenchmark);
BENCHMARK_MAIN();
