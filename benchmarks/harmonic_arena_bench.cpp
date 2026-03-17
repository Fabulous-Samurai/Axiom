#include <benchmark/benchmark.h>
#include <cstdlib>
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

BENCHMARK(BM_Malloc)->Arg(64)->Arg(256)->Arg(1024);
BENCHMARK(BM_HarmonicArena)->Arg(64)->Arg(256)->Arg(1024);

BENCHMARK_MAIN();
