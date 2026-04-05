// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file crash_dump.h
 * @brief Phase 3: Zero-Jitter Crash Vault using mmap-backed SPSC buffer
 *
 * Provides an "Invisible Telemetry" layer that persists metrics across crashes
 * without introducing OS-level jitter or blocking I/O.
 */

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>

namespace AXIOM {

/**
 * @brief Crash event record (Optimized for 64-byte cache alignment)
 */
struct alignas(64) CrashRecord {
  uint64_t timestamp_rdtsc;
  uint32_t event_id;
  uint32_t thread_id;
  char message[40];  // Fixed-size to avoid heap allocations
};

/**
 * @brief Lock-Free SPSC Crash Vault (mmap-backed)
 */
class CrashVault {
 public:
  static constexpr size_t MAX_RECORDS = 65536;  // Power of 2 for fast masking
  static constexpr size_t VAULT_SIZE = MAX_RECORDS * sizeof(CrashRecord);

 private:
  struct VaultHeader {
    alignas(64) std::atomic<uint64_t> head;
    alignas(64) std::atomic<uint64_t> tail;
    uint64_t magic_signature;
  };

  void* mmap_base_{nullptr};
  VaultHeader* header_{nullptr};
  CrashRecord* records_{nullptr};
  size_t mapped_size_{0};

#ifdef _WIN32
  void* file_handle_{nullptr};
  void* mapping_handle_{nullptr};
#else
  int fd_{-1};
#endif

 public:
  CrashVault() = default;
  ~CrashVault();

  /**
   * @brief Initialize the memory-mapped vault
   */
  bool initialize(const std::string& path);

  /**
   * @brief Record a metric or crash marker (Hot-Path / Lock-Free)
   */
  void record(uint32_t event_id, const char* msg) noexcept;

  /**
   * @brief Singleton instance for global visibility
   */
  static CrashVault& instance();

 private:
  CrashVault(const CrashVault&) = delete;
  CrashVault& operator=(const CrashVault&) = delete;
};

// Global macro for high-speed crash/latent marking
#define AXIOM_CRASH_MARK(event_id, msg) \
  AXIOM::CrashVault::instance().record(event_id, msg)

}  // namespace AXIOM
