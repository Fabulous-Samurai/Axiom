// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file telemetry.h
 * @brief Phase G: Heisenberg-Defying Telemetry (Implementation Layer)
 */

#pragma once

#include <array>
#include <atomic>
#include <string>
#include <string_view>
#include <thread>

#include "axiom_export.h"
#include "pmu_orchestrator.h"
#include "telemetry_base.h"

namespace AXIOM {

/**
 * @brief Heisenberg-Defying Telemetry Scribe
 * Zero-overhead logger using SPSC lock-free ring buffer.
 */
class AXIOM_EXPORT TelemetryScribe {
 public:
  static constexpr size_t RING_BUFFER_SIZE = 65536;  // 64K entries

  TelemetryScribe() = default;
  ~TelemetryScribe() { shutdown(); }

  TelemetryScribe(const TelemetryScribe&) = delete;
  TelemetryScribe& operator=(const TelemetryScribe&) = delete;

  /**
   * @brief Records a telemetry event in O(1) time.
   */
  AXIOM_FORCE_INLINE void Record(TelemetryEventType type,
                                 uint32_t event_id) noexcept {
    size_t current_head = head_.load(std::memory_order_relaxed);
    size_t next_head = (current_head + 1) % RING_BUFFER_SIZE;

    if (next_head == tail_.load(std::memory_order_acquire)) [[unlikely]] {
      dropped_records_.fetch_add(1, std::memory_order_relaxed);
      return;
    }

    TelemetryRecord& rec = ring_buffer_[current_head];
    auto hw = PMUOrchestrator::instance().ReadContext();

    AXIOM_LFENCE();
    rec.timestamp = AXIOM_RDTSC();
    rec.hw_instructions = hw.instructions;
    rec.hw_cycles = hw.cycles;
    rec.hw_l1_misses = hw.l1_misses;
    rec.hw_br_misses = hw.br_misses;
    rec.event_id = event_id;
    rec.type = type;

    head_.store(next_head, std::memory_order_release);
  }

  bool start(const std::string& log_path);
  void log_throughput(double ops_per_sec);
  double read_throughput();
  void shutdown();

  static TelemetryScribe& instance();

 private:
  void scribe_loop();

  alignas(64) std::array<TelemetryRecord, RING_BUFFER_SIZE> ring_buffer_{};
  alignas(64) std::atomic<size_t> head_{0};
  alignas(64) std::atomic<size_t> tail_{0};

  std::atomic<bool> running_{false};
  std::jthread scribe_thread_;
  std::string log_file_path_;
  std::atomic<uint64_t> dropped_records_{0};
};

}  // namespace AXIOM

// Override core stub with real implementation
#undef AXIOM_TELEMETRY_MARK
#define AXIOM_TELEMETRY_MARK(type, id) \
  ::AXIOM::TelemetryScribe::instance().Record(type, id)
