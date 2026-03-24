// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once

#include "algebraic_parser.h"
#include <asmjit/x86.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

#include "axiom_export.h"

namespace AXIOM {

/**
 * @brief Signature for JIT-compiled arithmetic functions.
 * @param vars Input array of variable values.
 * @return Computed result.
 */
using JiffedFunc = double (*)(const double* vars);

/**
 * @brief Zenith JIT Compiler for AXIOM Equations.
 * 
 * Translates ExprNode AST into optimized x64 machine code using AsmJit.
 */
class AXIOM_EXPORT ZenithJIT {
public:
    ZenithJIT();
    ~ZenithJIT();

    /**
     * @brief Compile an expression to a native function.
     * @param root The root of the AST to compile.
     * @param var_map Mapping of variable names to indices in the input array.
     * @return Native function pointer, or nullptr on failure.
     */
    JiffedFunc Compile(const ExprNode* root, const std::unordered_map<std::string, int>& var_map);

private:
    asmjit::JitRuntime rt_;
    
    // Internal recursive compiler
    void CompileNode(asmjit::x86::Compiler& cc, asmjit::x86::Gp vars_ptr, const ExprNode* node, 
                     const std::unordered_map<std::string, int>& var_map,
                     ::asmjit::x86::Vec& out);
};

} // namespace AXIOM





