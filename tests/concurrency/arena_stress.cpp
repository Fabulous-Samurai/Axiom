#include <gtest/gtest.h>
#include "arena_allocator.h"
#include <thread>
#include <vector>
#include <atomic>
#include <random>
#include <cstring>

TEST(ArenaStress, MultiThreadedAllocDealloc) {
    const int num_threads = 16;
    const int iterations = 10000;
    AXIOM::HarmonicArena arena(1024 * 1024 * 10, 0); // 10MB arena

    std::vector<std::thread> threads;
    std::atomic<bool> start{false};

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&arena, &start, t, iterations]() {
            while (!start) std::this_thread::yield();
            
            std::random_device rd; // NOSONAR: Only for stress test jitter
            std::mt19937_64 gen(rd()); 
            std::uniform_int_distribution<> size_dis(8, 1024);

            for (int i = 0; i < iterations; ++i) {
                size_t size = size_dis(gen);
                void* ptr = arena.allocate(size);
                if (ptr) {
                    // Touch memory
                    ::memset(ptr, 0xAA, size);
                }
            }
        });
    }

    start = true;
    for (auto& t : threads) t.join();
    
    // Test passes if it doesn't crash or hang (deadlock)
}
