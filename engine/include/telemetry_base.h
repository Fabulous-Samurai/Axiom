// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file telemetry_base.h
 * @brief Base types and macros for Heisenberg-Defying Telemetry.
 * Located in CORE layer to prevent upward dependencies.
 */

#pragma once

#include <cstdint>
#include "cpu_optimization.h"

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

static_assert(sizeof(TelemetryRecord) == 64, "TelemetryRecord alignment failure!");

} // namespace AXIOM

// Stub macro for core-level use. Implementation is linked in higher layers.
#ifndef AXIOM_TELEMETRY_MARK
#define AXIOM_TELEMETRY_MARK(type, id)
#endif
