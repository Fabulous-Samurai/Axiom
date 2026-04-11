#include <benchmark/benchmark.h>
#include <vector>
#include <cstdlib>

static void BumpAllocatorBenchmark(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<void*> allocations;
        for (int i = 0; i < 100000; ++i) {
            void* ptr = malloc(64);
            benchmark::DoNotOptimize(ptr);
            allocations.push_back(ptr);
        }
        for (void* ptr : allocations) {
            free(ptr);
        }
    }
}

BENCHMARK(BumpAllocatorBenchmark);
BENCHMARK_MAIN();
