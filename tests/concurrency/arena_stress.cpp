#include <gtest/gtest.h>
#include "arena_allocator.h"
#include "secure_random.h"
#include <thread>
#include <vector>
#include <atomic>
#include <cstring>

TEST(ArenaStress, MultiThreadedAllocDealloc) {
    const int num_threads = 16;
    const int iterations = 10000;
    AXIOM::HarmonicArena arena(1024 * 1024 * 10, 0); // 10MB arena

    std::vector<std::jthread> threads;
    std::atomic<bool> start{false};

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&arena, &start, t, iterations]() {
            while (!start) std::this_thread::yield();

            for (int i = 0; i < iterations; ++i) {
                size_t size = (size_t)AXIOM::SecureRandom::range(8, 1024);
                void* ptr = arena.allocate(size);
                if (ptr) {
                    // Touch memory
                    ::memset(ptr, 0xAA, size);
                }
            }
        });
    }

    start = true;

    // Test passes if it doesn't crash or hang (deadlock)
}
