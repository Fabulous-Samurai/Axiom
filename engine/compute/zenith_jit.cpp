// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "zenith_jit.h"
#include <iostream>
#include <asmjit/core.h>

#if defined(ASMJIT_BUILD_X86)
#include <asmjit/x86.h>
#endif
#if defined(ASMJIT_BUILD_AARCH64) || defined(ASMJIT_BUILD_ARM)
#include <asmjit/arm.h>
#endif

namespace AXIOM {

ZenithJIT::ZenithJIT() : rt_([]() {
    asmjit::JitAllocator::CreateParams params;
    params.options = asmjit::JitAllocatorOptions::kUseDualMapping;
    return params;
}()) {}
ZenithJIT::~ZenithJIT() = default;

JiffedFunc ZenithJIT::Compile(const ExprNode* root, const SymbolTable& var_map) noexcept {
    if (!root) return nullptr;
    
    asmjit::CodeHolder code;
    code.init(rt_.environment());
    
    // Attach logger to capture disassembly
    asmjit::StringLogger logger;
    code.set_logger(&logger);

#if defined(ASMJIT_BUILD_X86)
    asmjit::x86::Compiler cc(&code);
    asmjit::FuncNode* func = cc.add_func(asmjit::FuncSignature::build<double, const double*>(asmjit::CallConvId::kCDecl));
    
    asmjit::x86::Gp vars_ptr = cc.new_gp(asmjit::TypeId::kIntPtr, "vars_ptr");
    func->set_arg(0, vars_ptr);
    
    asmjit::x86::Vec result = cc.new_xmm("result");
    
    CompileNodeX86(cc, vars_ptr, root, var_map, result);
    cc.ret(result);
    
    // [SECURITY]: Add software-level guard region (INT3 traps)
    for (int i = 0; i < 8; ++i) {
        cc.int3();
    }

    cc.end_func();
    cc.finalize();
#elif defined(ASMJIT_BUILD_AARCH64)
    asmjit::aarch64::Compiler cc(&code);
    asmjit::FuncNode* func = cc.add_func(asmjit::FuncSignature::build<double, const double*>(asmjit::CallConvId::kCDecl));
    
    asmjit::aarch64::Gp vars_ptr = cc.new_gp(asmjit::TypeId::kIntPtr, "vars_ptr");
    func->set_arg(0, vars_ptr);
    
    asmjit::aarch64::Vec result = cc.new_v("result");
    
    CompileNodeAArch64(cc, vars_ptr, root, var_map, result);
    cc.ret(result);

    // [SECURITY]: Add software-level guard region (BRK traps)
    for (int i = 0; i < 8; ++i) {
        cc.brk(0);
    }

    cc.end_func();
    cc.finalize();
#else
    return nullptr; // Architecture not supported
#endif
        
    // Save the disassembly
    last_disassembly_.clear();
    const char* dis_data = logger.data();
    while (dis_data && *dis_data && last_disassembly_.size() < last_disassembly_.capacity()) {
        last_disassembly_.push_back(*dis_data++);
    }

    JiffedFunc fn;
    // [SECURITY]: asmjit::JitRuntime::add() handles the transition from W to X.
    // Memory is allocated as W, then changed to RX (Read+Execute) to maintain W^X.
    if (rt_.add(&fn, &code) != asmjit::kErrorOk) return nullptr;
    
    return fn;
}

JiffedMatrixFunc ZenithJIT::CompileMatrix(const ExprNode* root, const SymbolTable& var_map) noexcept {
    auto start_time = std::chrono::high_resolution_clock::now();

    if (!root) return nullptr;
    
    asmjit::CodeHolder code;
    code.init(rt_.environment());
    
    asmjit::StringLogger logger;
    code.set_logger(&logger);

#if defined(ASMJIT_BUILD_X86)
    asmjit::x86::Compiler cc(&code);
    // Signature: void (*)(const double* vars, double* out_matrix)
    asmjit::FuncNode* func = cc.add_func(asmjit::FuncSignature::build<void, const double*, double*>(asmjit::CallConvId::kCDecl));
    
    asmjit::x86::Gp vars_ptr = cc.new_gp(asmjit::TypeId::kIntPtr, "vars_ptr");
    asmjit::x86::Gp out_ptr = cc.new_gp(asmjit::TypeId::kIntPtr, "out_ptr");
    func->set_arg(0, vars_ptr);
    func->set_arg(1, out_ptr);
    
    AXIOM::FixedVector<asmjit::x86::Vec, 256> results;
    
    if (!root->CompileMatrixX86(cc, vars_ptr, var_map, results)) return nullptr;
    
    // Write results to out_ptr
    for (size_t i = 0; i < results.size(); ++i) {
        cc.vmovupd(asmjit::x86::ptr(out_ptr, (int)(i * 32)), results[i]);
    }
    
    cc.ret();
    cc.end_func();
    cc.finalize();

#elif defined(ASMJIT_BUILD_AARCH64)
    asmjit::aarch64::Compiler cc(&code);
    asmjit::FuncNode* func = cc.add_func(asmjit::FuncSignature::build<void, const double*, double*>(asmjit::CallConvId::kCDecl));
    
    asmjit::aarch64::Gp vars_ptr = cc.new_gp(asmjit::TypeId::kIntPtr, "vars_ptr");
    asmjit::aarch64::Gp out_ptr = cc.new_gp(asmjit::TypeId::kIntPtr, "out_ptr");
    func->set_arg(0, vars_ptr);
    func->set_arg(1, out_ptr);
    
    AXIOM::FixedVector<asmjit::aarch64::Vec, 256> results;
    
    if (!root->CompileMatrixAArch64(cc, vars_ptr, var_map, results)) return nullptr;
    
    // Write results to out_ptr (ARM NEON store)
    for (size_t i = 0; i < results.size(); ++i) {
        cc.st1(results[i].v4d(), asmjit::aarch64::ptr(out_ptr)); // Need proper offset mapping for production
        cc.add(out_ptr, out_ptr, 32);
    }
    
    cc.ret();
    cc.end_func();
    cc.finalize();
#else
    return nullptr;
#endif

    // Save the disassembly
    last_disassembly_.clear();
    const char* dis_data = logger.data();
    while (dis_data && *dis_data && last_disassembly_.size() < last_disassembly_.capacity()) {
        last_disassembly_.push_back(*dis_data++);
    }

    JiffedMatrixFunc fn;
    if (rt_.add(&fn, &code) != asmjit::kErrorOk) return nullptr;

    auto end_time = std::chrono::high_resolution_clock::now();
    last_compile_time_ms_ = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    global_last_compile_time_ms_ = last_compile_time_ms_;
    
    return fn;
}

#if defined(ASMJIT_BUILD_X86)
void ZenithJIT::CompileNodeX86(asmjit::x86::Compiler& cc, asmjit::x86::Gp vars_ptr, const ExprNode* node,
                           const SymbolTable& var_map,
                           asmjit::x86::Vec& out) noexcept {
    if (node) {
        node->CompileX86(cc, vars_ptr, var_map, out);
    }
}
#endif

#if defined(ASMJIT_BUILD_AARCH64)
void ZenithJIT::CompileNodeAArch64(asmjit::aarch64::Compiler& cc, asmjit::aarch64::Gp vars_ptr, const ExprNode* node,
                           const SymbolTable& var_map,
                           asmjit::aarch64::Vec& out) noexcept {
    if (node) {
        node->CompileAArch64(cc, vars_ptr, var_map, out);
    }
}
#endif

} // namespace AXIOM
