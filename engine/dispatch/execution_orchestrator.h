/**
 * @file execution_orchestrator.h
 * @brief High-performance, zero-copy operation dispatcher for AXIOM Engine.
 *
 * Utilizes the Pimpl idiom to provide a stable ABI, isolate heavy template
 * instantiations (e.g., Eigen), and enforce a compilation firewall.
 * Renamed from SelectiveDispatcher to reflect its true role.
 */

#pragma once

#ifndef EXECUTION_ORCHESTRATOR_H
#define EXECUTION_ORCHESTRATOR_H

#include <Eigen/Dense>
#include <memory>
#include <string>
#include <vector>

#include "axiom_status.h"
#include "dynamic_calc_types.h"

namespace AXIOM {

/**
 * @brief Represents a compute task that can be executed on CPU or GPU.
 */
struct ComputeTask {
  std::string_view operation;
  OperationComplexity complexity;
  bool async = false;
  uint64_t deadline_rdtsc = 0;
};

class ExecutionOrchestrator {
 public:
  explicit ExecutionOrchestrator();
  ~ExecutionOrchestrator();

  // Prevent copying
  ExecutionOrchestrator(const ExecutionOrchestrator&) = delete;
  ExecutionOrchestrator& operator=(const ExecutionOrchestrator&) = delete;

  /**
   * @brief High-performance dispatch with optional GPU offloading via Vulkan
   * SSBO.
   */
  EngineResult DispatchOperation(
      std::string_view operation,
      const AXIOM::FixedVector<std::string_view, 16>& args = {},
      ComputeEngine preferred_engine = ComputeEngine::Auto);

  /**
   * @brief Matrix operation with direct memory mapping for Vulkan kernels.
   */
  EngineResult DispatchMatrixOperation(
      std::string_view operation,
      const Eigen::Ref<const Eigen::MatrixXd>& matrix_data);

  /**
   * @brief Initiates an asynchronous compute offload.
   * @return Task ID for tracking status and results.
   */
  uint64_t OffloadToGPU(const ComputeTask& task, const void* data, size_t size);

  void SetPreferredEngine(ComputeEngine engine);
  void EnableFallback(bool enable = true);
  std::string_view GetPerformanceReport() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace AXIOM

#endif  // EXECUTION_ORCHESTRATOR_H
