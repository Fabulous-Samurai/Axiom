#include <benchmark/benchmark.h>
#include "ingress.h"
#include "arena_allocator.h"

static void BM_MockIngress(benchmark::State& state) {
    AXIOM::HarmonicArena arena(1024 * 1024, 0);
    auto ingress = AXIOM::IngressFactory::create(AXIOM::IngressFactory::ProviderType::MOCK_HFT_FEED, "mock0");
    ingress->start(&arena);

    for (auto _ : state) {
        ingress->poll([](const AXIOM::IngressFrame& frame) {
            auto len = frame.length;
            benchmark::DoNotOptimize(len);
        });
    }
    ingress->stop();
}
BENCHMARK(BM_MockIngress);

static void BM_AFXDPIngress(benchmark::State& state) {
    AXIOM::HarmonicArena arena(1024 * 1024, 0);
    // AF_XDP will gracefully fail on Windows/Unsupported Linux and return 0
    auto ingress = AXIOM::IngressFactory::create(AXIOM::IngressFactory::ProviderType::AF_XDP, "eth0");
    if (!ingress->start(&arena)) {
        state.SkipWithError("AF_XDP not supported on this platform/kernel");
        return;
    }

    for (auto _ : state) {
        ingress->poll([](const AXIOM::IngressFrame& frame) {
            auto len = frame.length;
            benchmark::DoNotOptimize(len);
        });
    }
    ingress->stop();
}
BENCHMARK(BM_AFXDPIngress);

BENCHMARK_MAIN();
