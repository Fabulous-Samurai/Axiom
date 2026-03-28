/**
 * @file execution_orchestrator_capi.h
 * @brief C Application Binary Interface (ABI) for AXIOM Execution Orchestrator
 * 
 * Renamed from selective_dispatcher_capi.h for architectural consistency.
 */

#pragma once

#ifndef EXECUTION_ORCHESTRATOR_CAPI_H
#define EXECUTION_ORCHESTRATOR_CAPI_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// OPAQUE POINTER
typedef void* AxiomExecutionHandle;

// --- LIFECYCLE ---
AXIOM_EXPORT AxiomExecutionHandle AxiomExecution_Create();
AXIOM_EXPORT void AxiomExecution_Destroy(AxiomExecutionHandle handle);

// --- ZERO-COPY DATA BRIDGE ---
AXIOM_EXPORT int AxiomExecution_DispatchMatrix(AxiomExecutionHandle handle, 
                                              const char* operation_name, 
                                              const double* raw_data, 
                                              size_t rows, 
                                              size_t cols);

#ifdef __cplusplus
}
#endif

#endif // EXECUTION_ORCHESTRATOR_CAPI_H
