// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file cognitive_commander.h
 * @brief Phase F: Cognitive Commander & SLM Integration
 *
 * Manages model weights in pinned physical memory and executes inferences in
 * pre-allocated tensor pools.
 */

#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "arena_allocator.h"
#include "fixed_vector.h"
#include "memory_pool.h"

namespace AXIOM {

/**
 * @brief Cognitive Commander for Small Language Model (SLM) isolation
 */
class CognitiveCommander {
  std::unique_ptr<PinnedMemoryPool> weight_pool_;
  std::unique_ptr<PinnedMemoryPool> activation_pool_;

 public:
  CognitiveCommander() = default;

  /**
   * @brief Load model weights into pinned memory
   */
  bool load_weights(const std::string& model_path);

  /**
   * @brief Perform inference using pre-allocated pools
   */
  void run_inference(const FixedVector<float, 1024>& input,
                     FixedVector<float, 1024>& output);
};

}  // namespace AXIOM
