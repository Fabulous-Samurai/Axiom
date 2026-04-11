// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file lock_free_ring_buffer.h
 * @brief High-performance Single-Producer Single-Consumer (SPSC) Lock-Free Ring
 * Buffer.
 */

#pragma once

#include <array>
#include <atomic>
#include <cstddef>

#include "cpu_optimization.h"

namespace AXIOM {

/**
 * @brief SPSC Lock-Free Ring Buffer
 * @tparam T Type of elements to store.
 * @tparam Capacity Total capacity (must be power of two for performance).
 */
template <typename T, size_t Capacity = 1024>
class SPSCQueue {
 public:
  static_assert((Capacity & (Capacity - 1)) == 0,
                "Capacity must be a power of two.");

  SPSCQueue() : head_(0), tail_(0) {}

  /**
   * @brief Push an item into the queue. Only the producer thread calls this.
   * @return true if successful, false if full.
   */
  bool push(const T& item) noexcept {
    const size_t h = head_.load(std::memory_order_relaxed);
    if ((h + 1) % Capacity == tail_.load(std::memory_order_acquire)) {
      return false;  // Full
    }
    buffer_[h] = item;
    head_.store((h + 1) % Capacity, std::memory_order_release);
    return true;
  }

  bool push(T&& item) noexcept {
    const size_t h = head_.load(std::memory_order_relaxed);
    if ((h + 1) % Capacity == tail_.load(std::memory_order_acquire)) {
      return false;  // Full
    }
    buffer_[h] = std::move(item);
    head_.store((h + 1) % Capacity, std::memory_order_release);
    return true;
  }

  /**
   * @brief Pop an item from the queue. Only the consumer thread calls this.
   * @return true if successful, false if empty.
   */
  bool pop(T& item) noexcept {
    const size_t t = tail_.load(std::memory_order_relaxed);
    if (t == head_.load(std::memory_order_acquire)) {
      return false;  // Empty
    }
    item = std::move(buffer_[t]);
    tail_.store((t + 1) % Capacity, std::memory_order_release);
    return true;
  }

  bool empty() const noexcept {
    return head_.load(std::memory_order_acquire) ==
           tail_.load(std::memory_order_acquire);
  }

  size_t size() const noexcept {
    size_t h = head_.load(std::memory_order_acquire);
    size_t t = tail_.load(std::memory_order_acquire);
    if (h >= t) return h - t;
    return Capacity - (t - h);
  }

 private:
  std::array<T, Capacity> buffer_;

  AXIOM_ALIGN_CACHE std::atomic<size_t> head_;
  AXIOM_ALIGN_CACHE std::atomic<size_t> tail_;
};

}  // namespace AXIOM
