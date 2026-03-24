// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "zenith_jit.h"
#include <iostream>
#include <asmjit/core.h>
#include <asmjit/x86.h>

namespace AXIOM {

ZenithJIT::ZenithJIT() = default;
ZenithJIT::~ZenithJIT() = default;

JiffedFunc ZenithJIT::Compile(const ExprNode* root, const std::unordered_map<std::string, int>& var_map) {
    if (!root) return nullptr;
    
    asmjit::CodeHolder code;
    code.init(rt_.environment());
    
    asmjit::x86::Compiler cc(&code);
    asmjit::FuncNode* func = cc.add_func(asmjit::FuncSignature::build<double, const double*>(asmjit::CallConvId::kCDecl));
    
    asmjit::x86::Gp vars_ptr = cc.new_gp(asmjit::TypeId::kIntPtr, "vars_ptr");
    func->set_arg(0, vars_ptr);
    
    asmjit::x86::Vec result = cc.new_xmm("result");
    
    try {
        CompileNode(cc, vars_ptr, root, var_map, result);
        cc.ret(result);
        
        // [SECURITY]: Add software-level guard region (INT3 traps)
        for (int i = 0; i < 8; ++i) {
            cc.int3();
        }

        cc.end_func();
        cc.finalize();
    } catch (...) {
        return nullptr;
    }

    JiffedFunc fn;
    // rt_.add() internally handles the W^X transition:
    // 1. Allocate RX or RW memory.
    // 2. Write code.
    // 3. (On some platforms/configs) Mark RX.
    if (rt_.add(&fn, &code) != asmjit::kErrorOk) return nullptr;
    
    return fn;
}

void ZenithJIT::CompileNode(asmjit::x86::Compiler& cc, asmjit::x86::Gp vars_ptr, const ExprNode* node,
                           const std::unordered_map<std::string, int>& var_map,
                           asmjit::x86::Vec& out) {
    if (!node || !node->Compile(cc, vars_ptr, var_map, out)) throw std::runtime_error("JIT error");
}
} // namespace AXIOM

