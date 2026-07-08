// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file axiom_bridge.h
 * @brief Zero-Allocation C-ABI for Project AXIOM GUI (Operation BRIDGE ALIGN)
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "axiom_export.h"

#ifdef __cplusplus
#define AXIOM_C_ALIGN(x) alignas(x)
#elif defined(_MSC_VER)
#define AXIOM_C_ALIGN(x) __declspec(align(x))
#else
#define AXIOM_C_ALIGN(x) __attribute__((aligned(x)))
#endif

#ifdef __cplusplus
namespace AXIOM {
extern "C" {
#endif

/**
 * @brief Calculation Result Schema for UI Integration
 * [SONAR WAVE 1]: Kept char[] for error_msg to maintain C-ABI compatibility.
 * Internal hot-paths utilize AXIOM::FixedVector.
 */
typedef struct AXIOM_C_ALIGN(64) {
  double value;
  int32_t status_code;
  uint64_t latency_rdtsc;
  char error_msg[128];
} Axiom_CalculationResult;

typedef struct AXIOM_C_ALIGN(64) {
  float x, y, z;
  uint32_t color;  // Packed RGBA
} Axiom_Point3D;

typedef struct AXIOM_C_ALIGN(64) {
  Axiom_Point3D position;
  uint32_t node_id;
  uint32_t parent_id;
  float cost_f;
  float cost_g;
} Axiom_MantisNode;

/**
 * @brief Execute a calculation via the AXIOM Engine.
 */
AXIOM_EXPORT Axiom_CalculationResult Axiom_Execute(const char* expression,
                                                   const char* mode);

/**
 * @brief Fetches disassembly for a JIT-compiled expression.
 */
AXIOM_EXPORT uintptr_t AxiomJit_GetDisassembly(const uint8_t* code_ptr,
                                               size_t size, char* out_buffer,
                                               size_t max_len);

/**
 * @brief Stream Mantis A* search tree nodes for Vulkan rendering.
 */
AXIOM_EXPORT size_t AxiomMantis_StreamSearchTree(Axiom_MantisNode* out_nodes,
                                                 size_t max_count);

#ifdef __cplusplus
}
}  // namespace AXIOM
#endif
