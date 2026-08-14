// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file memory_pool.h
 * @brief Centralized Hardware-Pinned Memory Pool for AXIOM Zenith.
 *
 * [MANDATORY PATH]: Provides VirtualLock/mlock guarantees for model weights.
 */

#pragma once

#include <atomic>
#include <cstddef>

#include "axiom_export.h"

namespace AXIOM {

/**
 * @brief High-performance pinned memory pool with bump-allocator.
 * Designed for Phase F SLM weights and Phase 7 SSBO mapping.
 */
class AXIOM_EXPORT PinnedMemoryPool {
 public:
  explicit PinnedMemoryPool(size_t size) noexcept;
  ~PinnedMemoryPool();

  // Prevent copying
  PinnedMemoryPool(const PinnedMemoryPool&) = delete;
  PinnedMemoryPool& operator=(const PinnedMemoryPool&) = delete;

  /**
   * @brief Pins the entire memory region to physical RAM.
   * @return true if successful (OS-level lock achieved).
   */
  bool pin() noexcept;
  void unpin() noexcept;

  /**
   * @brief Allocates memory from the pinned region.
   * @return Cache-aligned pointer, or nullptr if exhausted.
   */
  void* allocate(size_t size) noexcept;

  size_t capacity() const noexcept { return size_; }
  size_t used() const noexcept {
    return offset_.load(std::memory_order_relaxed);
  }

 private:
  void* base_addr_ = nullptr;
  size_t size_;
  std::atomic<size_t> offset_{0};
};

}  // namespace AXIOM
