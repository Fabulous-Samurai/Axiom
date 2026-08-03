// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#include "zenith_jit.h"

#include <asmjit/core.h>

#include <iostream>

#if defined(ASMJIT_BUILD_X86)
#include <asmjit/x86.h>
#endif
#if defined(ASMJIT_BUILD_AARCH64) || defined(ASMJIT_BUILD_ARM)
#include <asmjit/arm.h>
#endif

namespace AXIOM {

double ZenithJIT::global_last_compile_time_ms_ = 0.0;

ZenithJIT::ZenithJIT()
    : rt_([]() {
        asmjit::JitAllocator::CreateParams params;
        params.options = asmjit::JitAllocatorOptions::kUseDualMapping;
        return params;
      }()) {}
ZenithJIT::~ZenithJIT() = default;

JiffedFunc ZenithJIT::Compile(NodePtr root,
                              const SymbolTable& var_map) noexcept {
  if (!root) return nullptr;

  asmjit::CodeHolder code;
  code.init(rt_.environment());
  asmjit::StringLogger logger;
  code.set_logger(&logger);

#if defined(ASMJIT_BUILD_X86)
  asmjit::x86::Compiler cc(&code);
  asmjit::FuncNode* func =
      cc.add_func(asmjit::FuncSignature::build<double, const double*>(
          asmjit::CallConvId::kCDecl));
  asmjit::x86::Gp vars_ptr = cc.new_gp(asmjit::TypeId::kIntPtr, "vars_ptr");
  func->set_arg(0, vars_ptr);
  asmjit::x86::Vec result = cc.new_xmm("result");

  CompileNodeX86(cc, vars_ptr, root, var_map, result);
  cc.ret(result);
  cc.end_func();
  cc.finalize();
#elif defined(ASMJIT_BUILD_AARCH64)
  asmjit::aarch64::Compiler cc(&code);
  asmjit::FuncNode* func =
      cc.add_func(asmjit::FuncSignature::build<double, const double*>(
          asmjit::CallConvId::kCDecl));
  asmjit::aarch64::Gp vars_ptr = cc.new_gp(asmjit::TypeId::kIntPtr, "vars_ptr");
  func->set_arg(0, vars_ptr);
  asmjit::aarch64::Vec result = cc.new_v("result");

  CompileNodeAArch64(cc, vars_ptr, root, var_map, result);
  cc.ret(result);
  cc.end_func();
  cc.finalize();
#endif

  last_disassembly_.clear();
  const char* dis_data = logger.data();
  while (dis_data && *dis_data &&
         last_disassembly_.size() < last_disassembly_.capacity()) {
    last_disassembly_.push_back(*dis_data++);
  }

  JiffedFunc fn;
  if (rt_.add(&fn, &code) != asmjit::kErrorOk) return nullptr;
  return fn;
}

JiffedMatrixFunc ZenithJIT::CompileMatrix(NodePtr root,
                                          const SymbolTable& var_map) noexcept {
  // Matrix compilation recovery for Operation VARIANT SHIFT
  return nullptr;
}

#if defined(ASMJIT_BUILD_X86)
void ZenithJIT::CompileNodeX86(asmjit::x86::Compiler& cc,
                               asmjit::x86::Gp vars_ptr, NodePtr node,
                               const SymbolTable& var_map,
                               asmjit::x86::Vec& out) noexcept {
  if (!node) return;

  std::visit(
      [&](auto& n) {
        using T = std::decay_t<decltype(n)>;

        if constexpr (std::is_same_v<T, NumberNode>) {
          asmjit::x86::Gp constant_ptr = cc.new_gp(asmjit::TypeId::kIntPtr);
          cc.mov(constant_ptr, (uintptr_t)&n.value);
          cc.vmovsd(out, asmjit::x86::ptr(constant_ptr));
        } else if constexpr (std::is_same_v<T, VariableNode>) {
          int idx = LookupSymbol(var_map, n.name);
          if (idx >= 0) {
            cc.vmovsd(out, asmjit::x86::ptr(vars_ptr, idx * 8));
          }
        } else if constexpr (std::is_same_v<T, BinaryOpNode>) {
          asmjit::x86::Vec left_res = cc.new_xmm();
          asmjit::x86::Vec right_res = cc.new_xmm();
          CompileNodeX86(cc, vars_ptr, n.left, var_map, left_res);
          CompileNodeX86(cc, vars_ptr, n.right, var_map, right_res);

          switch (n.op) {
            case '+':
              cc.vaddsd(out, left_res, right_res);
              break;
            case '-':
              cc.vsubsd(out, left_res, right_res);
              break;
            case '*':
              cc.vmulsd(out, left_res, right_res);
              break;
            case '/':
              cc.vdivsd(out, left_res, right_res);
              break;
          }
        }
        // ... (More cases for other node types)
      },
      *node);
}
#endif

#if defined(ASMJIT_BUILD_AARCH64)
void ZenithJIT::CompileNodeAArch64(asmjit::aarch64::Compiler& cc,
                                   asmjit::aarch64::Gp vars_ptr, NodePtr node,
                                   const SymbolTable& var_map,
                                   asmjit::aarch64::Vec& out) noexcept {
  if (!node) return;

  std::visit(
      [&](auto& n) {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, NumberNode>) {
          asmjit::aarch64::Gp constant_ptr = cc.new_gp(asmjit::TypeId::kIntPtr);
          cc.mov(constant_ptr, (uintptr_t)&n.value);
          cc.ldr(out, asmjit::aarch64::ptr(constant_ptr));
        }
        // ... (Implement other AArch64 cases similar to x86)
      },
      *node);
}
#endif

}  // namespace AXIOM
