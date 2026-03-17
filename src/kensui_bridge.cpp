#include "../include/kensui_bridge.h"
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include "arena_allocator.h"

namespace AXIOM {


// Mock implementation for JIT Disassembly
// In a full implementation, this would use Zydis or AsmJit's built-in disassembler.
AXIOM_EXPORT uintptr_t AxiomJit_GetDisassembly(const uint8_t* code_ptr, size_t size, char* out_preallocated_buffer, size_t max_len) {
    if (!out_preallocated_buffer || max_len == 0) return 0;

    int offset = snprintf(out_preallocated_buffer, max_len, "JIT_DISASM (size=%zu): ", size);
    if (offset < 0) return 0;
    
    for (size_t i = 0; i < std::min(size, (size_t)16); ++i) {
        if ((size_t)offset >= max_len - 1) break;
        offset += snprintf(out_preallocated_buffer + offset, max_len - (size_t)offset, "%02X ", code_ptr[i]);
    }

    if (size > 16 && (size_t)offset < max_len - 1) {
        snprintf(out_preallocated_buffer + offset, max_len - (size_t)offset, "...");
    }

    return (uintptr_t)strlen(out_preallocated_buffer);
}

// Mock implementation for Mantis A* Search Tree
// This will be hooked into the real solver state later.
AXIOM_EXPORT size_t AxiomMantis_StreamSearchTree(Kensui_MantisNode* out_nodes, size_t max_count) {
    if (!out_nodes || max_count == 0) return 0;

    // Return dummy node data for integration testing
    size_t count = std::min(max_count, (size_t)5);
    for (size_t i = 0; i < count; ++i) {
        out_nodes[i].node_id = (uint32_t)i;
        out_nodes[i].parent_id = (i == 0) ? 0xFFFFFFFF : (uint32_t)(i - 1);
        // Explicitly cast to Kensui_Point3D to resolve compiler ambiguity
        out_nodes[i].position = Kensui_Point3D{(float)i * 2.0f, (float)std::sin((float)i) * 2.0f, (float)i * -1.0f};
        out_nodes[i].position.color = 0xFF00FF00; // Green nodes
        out_nodes[i].cost_f = (float)i * 1.5f;
        out_nodes[i].cost_g = (float)i * 1.0f;
    }

    return count;
}





} // namespace AXIOM
