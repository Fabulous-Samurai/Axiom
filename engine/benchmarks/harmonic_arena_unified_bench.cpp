#include <benchmark/benchmark.h>
#include <cstdlib>
#include <vector>
#include "harmonic_arena.h"

// --- Malloc Baseline ---
static void BM_Malloc(benchmark::State& state) {
    for (auto _ : state) {
        void* ptr = std::malloc(state.range(0));
        benchmark::DoNotOptimize(ptr);
        std::free(ptr);
    }
}

// --- Harmonic Arena Test ---
static void BM_HarmonicArena(benchmark::State& state) {
    HarmonicArena arena(state.range(0) * state.iterations()); // Pre-size arena
    for (auto _ : state) {
        void* ptr = arena.allocate(state.range(0));
        benchmark::DoNotOptimize(ptr);
        // Deallocate is a no-op in HarmonicArena's typical lifecycle, 
        // so we don't benchmark it to reflect real usage.
    }
}

// --- Vector Baseline (No Reserve) ---
static void BM_VectorPushBack(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> vec;
        for (int i = 0; i < state.range(0); ++i) {
            vec.push_back(i);
            benchmark::DoNotOptimize(vec.data());
        }
    }
}

// --- Vector Baseline (With Reserve) ---
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

// Argument definitions
BENCHMARK(BM_Malloc)->DenseRange(64, 1024, 64);
BENCHMARK(BM_HarmonicArena)->DenseRange(64, 1024, 64);
BENCHMARK(BM_VectorPushBack)->DenseRange(100, 1000, 100);
BENCHMARK(BM_VectorReservePushBack)->DenseRange(100, 1000, 100);

BENCHMARK_MAIN();
