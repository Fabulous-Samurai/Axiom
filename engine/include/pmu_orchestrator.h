// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file pmu_orchestrator.h
 * @brief Phase G: Power Management Unit (PMU) Orchestration for Low-Latency Profiling.
 */

#pragma once

#include "cpu_optimization.h"
#include <cstdint>
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #include <immintrin.h>
#endif

namespace AXIOM {

/**
 * @brief Hardware Counter Indices for rdpmc
 */
struct PMUMetrics {
    uint64_t instructions;
    uint64_t cycles;
    uint64_t l1_misses;
    uint64_t br_misses;
};

/**
 * @brief PMUOrchestrator: Manages hardware performance counters.
 *
 * [MANDATORY PATH]: Uses perf_event_open + mmap to authorize user-mode rdpmc.
 */
class PMUOrchestrator {
public:
    static PMUOrchestrator& instance() noexcept;

    bool Initialize() noexcept;
    void Shutdown() noexcept;

    /**
     * @brief Reads hardware counters with OoOE serialization.
     *
     * [MANDATORY PATH]: _mm_lfence() used before every rdpmc.
     */
    AXIOM_FORCE_INLINE PMUMetrics ReadContext() const noexcept {
        PMUMetrics metrics;

        // Serialize pipeline
        AXIOM_LFENCE();

        #if defined(__linux__)
            // On Linux, indices are determined by mmap'd perf_event page.
            metrics.instructions = ReadCounter(0);
            metrics.cycles = ReadCounter(1);
            metrics.l1_misses = ReadCounter(2);
            metrics.br_misses = ReadCounter(3);
        #else
            // Fallback or Windows implementation
            metrics.instructions = 0;
            metrics.cycles = AXIOM_RDTSC();
            metrics.l1_misses = 0;
            metrics.br_misses = 0;
        #endif

        return metrics;
    }

private:
    PMUOrchestrator() noexcept = default;
    ~PMUOrchestrator() noexcept { Shutdown(); }

    uint64_t ReadCounter(int idx) const noexcept;

    #if defined(__linux__)
    struct PerfEventPage {
        int fd;
        void* mmap_base;
    };
    PerfEventPage pages_[4];
    #endif

    bool initialized_{false};
};

} // namespace AXIOM
