#include <benchmark/benchmark.h>
#include <cstdlib>
#include <vector>
#include "harmonic_arena.h"

static void BM_Malloc(benchmark::State& state) {
    for (auto _ : state) {
        void* ptr = std::malloc(state.range(0));
        benchmark::DoNotOptimize(ptr);
        std::free(ptr);
    }
}

static void BM_HarmonicArena(benchmark::State& state) {
    HarmonicArena arena;
    for (auto _ : state) {
        void* ptr = arena.allocate(state.range(0));
        benchmark::DoNotOptimize(ptr);
        arena.deallocate(ptr);
    }
}

static void BM_VectorPushBack(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> vec;
        for (int i = 0; i < state.range(0); ++i) {
            vec.push_back(i);
            benchmark::DoNotOptimize(vec.data());
        }
    }
}

static void BM_VectorReservePushBack(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> vec;
        vec.reserve(state.range(0));
        for (int i = 0; i < state.range(0); ++i) {
            vec.push_back(i);
            benchmark::DoNotOptimize(vec.data());
        }
    }
}

BENCHMARK(BM_Malloc)->DenseRange(1, 50);
BENCHMARK(BM_HarmonicArena)->DenseRange(1, 50);
BENCHMARK(BM_VectorPushBack)->DenseRange(1, 50);
BENCHMARK(BM_VectorReservePushBack)->DenseRange(1, 50);

BENCHMARK_MAIN();
