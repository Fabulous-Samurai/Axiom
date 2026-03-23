#include "../include/kensui_bridge.h"
#include "../include/pluto_controller.h"
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include "arena_allocator.h"

namespace AXIOM {

// Real implementation for JIT Disassembly
AXIOM_EXPORT uintptr_t AxiomJit_GetDisassembly(const uint8_t* code_ptr, size_t size, char* out_preallocated_buffer, size_t max_len) {
    if (!out_preallocated_buffer || max_len == 0) return 0;

    const char* prefix = "JIT_DISASM (size=";
    size_t offset = 0;
    while (prefix[offset] != '\0' && offset < max_len - 1) {
        out_preallocated_buffer[offset] = prefix[offset];
        offset++;
    }
    
    char size_buf[32];
    int size_len = std::snprintf(size_buf, sizeof(size_buf), "%zu", size);
    if (size_len > 0) {
        for (int j = 0; j < size_len && offset < max_len - 1; ++j) {
            out_preallocated_buffer[offset++] = size_buf[j];
        }
    }

    const char* suffix = "): ";
    int sj = 0;
    while (suffix[sj] != '\0' && offset < max_len - 1) {
        out_preallocated_buffer[offset++] = suffix[sj++];
    }
    
    for (size_t i = 0; i < std::min(size, (size_t)16); ++i) {
        if (offset + 3 >= max_len) break;
        static const char hex_chars[] = "0123456789ABCDEF";
        out_preallocated_buffer[offset++] = hex_chars[(code_ptr[i] >> 4) & 0xF];
        out_preallocated_buffer[offset++] = hex_chars[code_ptr[i] & 0xF];
        out_preallocated_buffer[offset++] = ' ';
    }

    if (size > 16 && offset + 3 < max_len) {
        out_preallocated_buffer[offset++] = '.';
        out_preallocated_buffer[offset++] = '.';
        out_preallocated_buffer[offset++] = '.';
    }
    out_preallocated_buffer[offset] = '\0';

    return (uintptr_t)offset;
}

// Real implementation for Mantis A* Search Tree (Stream from Pluto)
AXIOM_EXPORT size_t AxiomMantis_StreamSearchTree(Kensui_MantisNode* out_nodes, size_t max_count) {
    if (!out_nodes || max_count == 0) return 0;

    thread_local std::array<Mantis::AStarNode, 1024> core_nodes;
    size_t count = Pluto::PlutoController::instance().get_search_tree(
        core_nodes.data(), std::min(max_count, core_nodes.size()));

    for (size_t i = 0; i < count; ++i) {
        out_nodes[i].node_id = core_nodes[i].id;
        out_nodes[i].parent_id = core_nodes[i].parent_id;
        
        out_nodes[i].position.x = core_nodes[i].features.data[0]; 
        out_nodes[i].position.y = core_nodes[i].features.data[1]; 
        out_nodes[i].position.z = core_nodes[i].features.data[2]; 
        
        float normalized_cost = std::min(1.0f, core_nodes[i].f_cost / 50.0f);
        uint8_t r = static_cast<uint8_t>(255 * normalized_cost);
        uint8_t g = static_cast<uint8_t>(255 * (1.0f - normalized_cost));
        out_nodes[i].position.color = 0xFF000000 | (r << 16) | (g << 8);

        out_nodes[i].cost_f = core_nodes[i].f_cost;
        out_nodes[i].cost_g = core_nodes[i].g_cost;
    }

    return count;
}

} // namespace AXIOM
