// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "../include/axiom_bridge.h"
#include "../include/pluto_controller.h"
#include "../include/dynamic_calc.h"
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <variant>

namespace AXIOM {

AXIOM_EXPORT Axiom_CalculationResult Axiom_Execute(const char* expression, const char* mode) {
    Axiom_CalculationResult res = {0.0, 0, 0, ""};
    if (!expression) {
        res.status_code = -1;
        std::strncpy(res.error_msg, "Null expression", 127);
        return res;
    }

    thread_local DynamicCalc engine;
    
    // Set mode
    std::string_view m(mode ? mode : "algebraic");
    if (m == "linear") engine.SetMode(CalculationMode::LINEAR_SYSTEM);
    else if (m == "stats") engine.SetMode(CalculationMode::STATISTICS);
    else if (m == "symbolic") engine.SetMode(CalculationMode::SYMBOLIC);
    else engine.SetMode(CalculationMode::ALGEBRAIC);

    auto start_rdtsc = AXIOM_RDTSC();
    auto engine_res = engine.Evaluate(expression);
    res.latency_rdtsc = AXIOM_RDTSC() - start_rdtsc;

    if (engine_res.HasErrors()) {
        res.status_code = 1;
        // Map error type to string
        if (engine_res.error.has_value()) {
            std::visit([&](auto& err) {
                std::snprintf(res.error_msg, 128, "Error Code: %d", (int)err);
            }, *engine_res.error);
        } else {
            std::strncpy(res.error_msg, "Unknown Engine Error", 127);
        }
    } else {
        auto val = engine_res.GetDouble();
        if (val) res.value = *val;
        res.status_code = 0;
    }

    return res;
}

AXIOM_EXPORT uintptr_t AxiomJit_GetDisassembly(const uint8_t* code_ptr, size_t size, char* out_preallocated_buffer, size_t max_len) {
    if (!out_preallocated_buffer || max_len == 0) return 0;
    // ... (rest of disassembly implementation)
    std::snprintf(out_preallocated_buffer, max_len, "AXIOM_JIT_BINARY (0x%p, %zu bytes)", code_ptr, size);
    return std::strlen(out_preallocated_buffer);
}

AXIOM_EXPORT size_t AxiomMantis_StreamSearchTree(Axiom_MantisNode* out_nodes, size_t max_count) {
    if (!out_nodes || max_count == 0) return 0;
    // Implementation details...
    return 0; 
}

} // namespace AXIOM
