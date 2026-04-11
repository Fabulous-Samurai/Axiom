// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file kensui_bridge.h
 * @brief Zero-Allocation C-ABI for Project AXIOM KENSUI GUI.
 *
 * [MANDATORY PATH]: NO std::string or heap allocations across this boundary.
 * GUI must provide pre-allocated buffers for all data transfers.
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
 * @brief Point structure for 3D Nav-Console (Mantis A*)
 */
typedef struct AXIOM_C_ALIGN(64) {
    float x, y, z;
    uint32_t color; // Packed RGBA
} Kensui_Point3D;

/**
 * @brief Decision Tree Node for Pluto (Mantis A*)
 */
typedef struct AXIOM_C_ALIGN(64) {
    Kensui_Point3D position;
    uint32_t node_id;
    uint32_t parent_id;
    float cost_f;
    float cost_g;
} Kensui_MantisNode;

/**
 * @brief Fetches disassembly for a JIT-compiled expression.
 *
 * @param code_ptr Point to the machine code in memory.
 * @param size Size of the machine code.
 * @param out_buffer Pre-allocated buffer provided by the UI.
 * @param max_len Maximum length of the output buffer.
 * @return Actual length of the string written.
 */
AXIOM_EXPORT uintptr_t AxiomJit_GetDisassembly(const uint8_t* code_ptr, size_t size, char* out_buffer, size_t max_len);

/**
 * @brief Stream Mantis A* search tree nodes for Vulkan rendering.
 *
 * @param out_nodes Pre-allocated array of Kensui_MantisNode.
 * @param max_count Maximum number of nodes the UI can accept.
 * @return Number of nodes actually written.
 */
AXIOM_EXPORT size_t AxiomMantis_StreamSearchTree(Kensui_MantisNode* out_nodes, size_t max_count);

#ifdef __cplusplus
}
} // namespace AXIOM
#endif
