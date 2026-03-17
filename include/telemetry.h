/**
 * @file telemetry.h
 * @brief Phase G: Heisenberg-Defying Telemetry (Lock-Free RDTSC SPSC)
 */

#pragma once

#include "cpu_optimization.h"
#include <atomic>
#include <string_view>
#include <array>
#include <thread>
#include <fstream>
#include <filesystem>

#include "pmu_orchestrator.h"

#include "axiom_export.h"

namespace AXIOM {

/**
 * @brief High-precision telemetry event types
 */
enum class TelemetryEventType : uint8_t {
    LATENCY_MARKER,
    ALLOCATION_EVENT,
    MODE_SWITCH,
    CIRCUIT_BREAKER_TRIGGER,
    CUSTOM_SIGNAL
};

/**
 * @brief Raw telemetry record (Exactly 1 Cache Line - 64 bytes)
 * 
 * [MANDATORY PATH]: static_assert enforced to prevent MESI invalidation storms.
 * Includes hardware performance counters for zero-interference profiling.
 */
struct alignas(64) TelemetryRecord {
    uint64_t timestamp;        // RDTSC cycle count (8)
    uint64_t hw_instructions;  // Retired instructions (8)
    uint64_t hw_cycles;        // Non-halted cycles (8)
    uint64_t hw_l1_misses;     // L1 Data cache misses (8)
    uint64_t hw_br_misses;     // Branch mispredictions (8)
    uint32_t event_id;         // Custom ID (4)
    TelemetryEventType type;   // Event type (1)
    uint8_t  padding[19];      // Pad to exactly 64 bytes (19)
};

static_assert(sizeof(TelemetryRecord) == 64, "TelemetryRecord MUST be exactly 1 Cache Line to prevent L1 Bank Conflicts/MESI throughput loss.");

/**
 * @brief Heisenberg-Defying Telemetry Scribe
 * Zero-overhead logger using SPSC lock-free ring buffer.
 */
class AXIOM_EXPORT TelemetryScribe {
public:
    static constexpr size_t RING_BUFFER_SIZE = 65536; // 64K entries
    
    TelemetryScribe() = default;
    ~TelemetryScribe() { shutdown(); }

    TelemetryScribe(const TelemetryScribe&) = delete;
    TelemetryScribe& operator=(const TelemetryScribe&) = delete;

    /**
     * @brief Records a telemetry event in O(1) time without blocking or system calls.
     * 
     * [MANDATORY PATH]: Uses _mm_lfence() and hardware PMUs for zero-probe accuracy.
     */
    AXIOM_FORCE_INLINE void Record(TelemetryEventType type, uint32_t event_id) noexcept {
        size_t current_head = head_.load(std::memory_order_relaxed);
        size_t next_head = (current_head + 1) % RING_BUFFER_SIZE;
        
        if (next_head == tail_.load(std::memory_order_acquire)) [[unlikely]] {
            dropped_records_.fetch_add(1, std::memory_order_relaxed);
            return;
        }

        TelemetryRecord& rec = ring_buffer_[current_head];

        // [MANDATORY PATH]: Hardware counter sampling with serialization
        auto hw = PMUOrchestrator::instance().ReadContext();
        
        // Final serialization before timestamp
        _mm_lfence();
        rec.timestamp = AXIOM_RDTSC;
        
        rec.hw_instructions = hw.instructions;
        rec.hw_cycles = hw.cycles;
        rec.hw_l1_misses = hw.l1_misses;
        rec.hw_br_misses = hw.br_misses;
        rec.event_id = event_id;
        rec.type = type;

        head_.store(next_head, std::memory_order_release);
    }

    bool start(std::string_view log_path);
    void shutdown();

    static TelemetryScribe& instance();

private:
    void scribe_loop();

    AXIOM_ALIGN_CACHE std::array<TelemetryRecord, RING_BUFFER_SIZE> ring_buffer_{};
    
    AXIOM_ALIGN_CACHE std::atomic<size_t> head_{0};
    AXIOM_ALIGN_CACHE std::atomic<size_t> tail_{0};
    
    std::atomic<bool> running_{false};
    std::jthread scribe_thread_;
    std::string log_file_path_;
    std::atomic<uint64_t> dropped_records_{0};
};

// Global telemetry marker macro for zero-overhead instrumentation
#define AXIOM_TELEMETRY_MARK(type, id) ::AXIOM::TelemetryScribe::instance().Record(type, id)

} // namespace AXIOM

