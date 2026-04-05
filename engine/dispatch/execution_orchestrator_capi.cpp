/**
 * @file execution_orchestrator_capi.cpp
 * @brief AXIOM FFI Boundary Implementation for ExecutionOrchestrator
 * [MANDATE]: Zenith Pillar Compliance (-fno-exceptions)
 */

#include "execution_orchestrator_capi.h"

#include <Eigen/Core>
#include <iostream>
#include <new>  // For std::nothrow

#include "execution_orchestrator.h"

using namespace AXIOM;

AXIOM_EXPORT AxiomExecutionHandle AxiomExecution_Create() {
  // In -fno-exceptions, we use nothrow new to return nullptr on failure
  auto* orchestrator = new (std::nothrow) ExecutionOrchestrator();
  return static_cast<void*>(orchestrator);
}

AXIOM_EXPORT void AxiomExecution_Destroy(AxiomExecutionHandle handle) {
  if (handle) {
    delete static_cast<ExecutionOrchestrator*>(handle);
  }
}

AXIOM_EXPORT int AxiomExecution_DispatchMatrix(AxiomExecutionHandle handle,
                                               const char* operation_name,
                                               const double* raw_data,
                                               size_t rows, size_t cols) {
  if (!handle || !operation_name || !raw_data) return -1;

  // Sanity check for matrix dimensions
  if (rows == 0 || cols == 0 || rows > 100000 || cols > 100000) return -1;

  auto* orchestrator = static_cast<ExecutionOrchestrator*>(handle);
  std::string_view op_str(operation_name);

  // Zero-copy mapping of raw buffer to Eigen Matrix
  Eigen::Map<const Eigen::MatrixXd> mapped_matrix(
      raw_data, static_cast<Eigen::Index>(rows),
      static_cast<Eigen::Index>(cols));

  EngineResult result =
      orchestrator->DispatchMatrixOperation(std::string(op_str), mapped_matrix);

  if (result.HasErrors()) {
    return -3;  // Specific error code for compute failure
  }

  return 0;  // Success
}
