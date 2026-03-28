// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once

#include "algebraic_parser.h"

// Undefine 'emit' as it's a keyword in Qt and a method in AsmJit
#ifdef emit
#undef emit
#endif

#include <asmjit/asmjit.h>
#if defined(ASMJIT_BUILD_X86)
#include <asmjit/x86.h>
#endif
#if defined(ASMJIT_BUILD_AARCH64) || defined(ASMJIT_BUILD_ARM)
#include <asmjit/arm.h>
#endif

#include <memory>

#include "axiom_export.h"
#include "fixed_vector.h"

namespace AXIOM {

/**
 * @brief Signature for JIT-compiled arithmetic functions.
 * @param vars Input array of variable values.
 * @return Computed result.
 */
using JiffedFunc = double (*)(const double* vars);

/**
 * @brief Signature for JIT-compiled matrix functions.
 * @param vars Input array of variable values.
 * @param out_matrix Output pointer where the matrix elements will be written.
 */
using JiffedMatrixFunc = void (*)(const double* vars, double* out_matrix);

/**
 * @brief Zenith JIT Compiler for AXIOM Equations.
 * 
 * Translates ExprNode AST into optimized x64 or AArch64 machine code using AsmJit.
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
    JiffedFunc Compile(const ExprNode* root, const SymbolTable& var_map) noexcept;

    /**
     * @brief Compile a matrix expression to a native function.
     * @param root The root of the AST to compile.
     * @param var_map Mapping of variable names to indices in the input array.
     * @return Native matrix function pointer, or nullptr on failure.
     */
    JiffedMatrixFunc CompileMatrix(const ExprNode* root, const SymbolTable& var_map) noexcept;

    /**
     * @brief Get the disassembly of the last compiled function.
     */
    const FixedVector<char, 32768>& last_disassembly() const noexcept { return last_disassembly_; }

    /**
     * @brief Get the time taken for the last compilation in milliseconds globally.
     */
    static double GetLastCompileTimeMs() noexcept { return global_last_compile_time_ms_; }

private:
    asmjit::JitRuntime rt_;
    FixedVector<char, 32768> last_disassembly_;
    double last_compile_time_ms_ = 0.0;
    static double global_last_compile_time_ms_;
    
#if defined(ASMJIT_BUILD_X86)
    // Internal recursive compiler for x86/x64
    void CompileNodeX86(asmjit::x86::Compiler& cc, asmjit::x86::Gp vars_ptr, const ExprNode* node, 
                     const SymbolTable& var_map,
                     ::asmjit::x86::Vec& out) noexcept;
#endif

#if defined(ASMJIT_BUILD_AARCH64)
    // Internal recursive compiler for AArch64
    void CompileNodeAArch64(asmjit::aarch64::Compiler& cc, asmjit::aarch64::Gp vars_ptr, const ExprNode* node, 
                     const SymbolTable& var_map,
                     asmjit::aarch64::Vec& out) noexcept;
#endif
};

} // namespace AXIOM





