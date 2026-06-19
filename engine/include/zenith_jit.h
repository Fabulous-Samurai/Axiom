// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#pragma once

#include "ast_nodes.h"  // Includes AnyNode and NodePtr

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

using JiffedFunc = double (*)(const double* vars);
using JiffedMatrixFunc = void (*)(const double* vars, double* out_matrix);

/**
 * @brief Zenith JIT Compiler (Variant-Aware)
 * Optimized for Static-Variant Dispatch.
 */
class AXIOM_EXPORT ZenithJIT {
 public:
  ZenithJIT();
  ~ZenithJIT();

  JiffedFunc Compile(NodePtr root, const SymbolTable& var_map) noexcept;
  JiffedMatrixFunc CompileMatrix(NodePtr root,
                                 const SymbolTable& var_map) noexcept;

  const FixedVector<char, 32768>& last_disassembly() const noexcept {
    return last_disassembly_;
  }
  static double GetLastCompileTimeMs() noexcept {
    return global_last_compile_time_ms_;
  }

 private:
  asmjit::JitRuntime rt_;
  FixedVector<char, 32768> last_disassembly_;
  double last_compile_time_ms_ = 0.0;
  static double global_last_compile_time_ms_;

#if defined(ASMJIT_BUILD_X86)
  void CompileNodeX86(asmjit::x86::Compiler& cc, asmjit::x86::Gp vars_ptr,
                      NodePtr node, const SymbolTable& var_map,
                      ::asmjit::x86::Vec& out) noexcept;
#endif

#if defined(ASMJIT_BUILD_AARCH64)
  void CompileNodeAArch64(asmjit::aarch64::Compiler& cc,
                          asmjit::aarch64::Gp vars_ptr, NodePtr node,
                          const SymbolTable& var_map,
                          asmjit::aarch64::Vec& out) noexcept;
#endif
};

}  // namespace AXIOM
