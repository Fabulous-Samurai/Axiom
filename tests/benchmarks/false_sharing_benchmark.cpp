#include <benchmark/benchmark.h>
#include <atomic>
#include <thread>
#include <vector>

alignas(64) std::atomic<int> counter1{0};
alignas(64) std::atomic<int> counter2{0};

static void FalseSharingBenchmark(benchmark::State& state) {
    for (auto _ : state) {
        std::thread t1([] {
            for (int i = 0; i < 1000000; ++i) {
                counter1.fetch_add(1, std::memory_order_relaxed);
            }
        });

        std::thread t2([] {
            for (int i = 0; i < 1000000; ++i) {
                counter2.fetch_add(1, std::memory_order_relaxed);
            }
        });

        t1.join();
        t2.join();
    }
}

BENCHMARK(FalseSharingBenchmark);
BENCHMARK_MAIN();
