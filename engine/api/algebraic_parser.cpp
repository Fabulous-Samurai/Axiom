// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "ast_nodes.h"
#include "../include/algebraic_parser.h"
#include "../include/string_helpers.h"
#include <exception>
#include <iostream>
#include <asmjit/core.h>
#if defined(ASMJIT_BUILD_X86)
    #include <asmjit/x86.h>
#endif
#if defined(ASMJIT_BUILD_AARCH64)
    #include <asmjit/arm.h>
#endif
#include "../include/telemetry.h"
#include <cmath>
#define SLEEF_STATIC_LIBS
#include <sleef.h>
#include <numbers>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <format>
#include <shared_mutex>
#include <mutex>
#include "arena_allocator.h"
#include <cstdio>
#include <cctype>

namespace AXIOM {

namespace {
    constexpr double PI_CONST = ::std::numbers::pi;
    constexpr double D2R = ::std::numbers::pi / 180.0;
    constexpr double R2D = 180.0 / ::std::numbers::pi;

    static double jit_abs(double x) { return ::std::abs(x); }
    static double jit_exp(double x) { return ::std::exp(x); }
    static double jit_log10(double x) { return ::std::log10(x); }
    static double jit_log(double x) { return ::std::log(x); }
    static double jit_sin(double x) { return ::std::sin(x); }
    static double jit_cos(double x) { return ::std::cos(x); }
    static double jit_tan(double x) { return ::std::tan(x); }
    static double jit_pow(double x, double y) { return ::std::pow(x, y); }

    struct ParserState {
        ::std::string_view input;
        size_t pos{0};
        Arena& arena;
        int depth{0};
        static constexpr int MAX_DEPTH = 512;

        void skip_ws() noexcept {
            while (pos < input.length() && ::std::isspace(static_cast<unsigned char>(input[pos]))) pos++;
        }

        bool has_more() const noexcept { return pos < input.length(); }
        char peek() const noexcept { return pos < input.length() ? input[pos] : '\0'; }
        char get() noexcept { return pos < input.length() ? input[pos++] : '\0'; }

        bool match(char expected) noexcept {
            skip_ws();
            if (peek() == expected) {
                pos++;
                return true;
            }
            return false;
        }

        struct DepthGuard {
            ParserState& s;
            bool ok;
            DepthGuard(ParserState& state) noexcept : s(state) {
                s.depth++;
                ok = s.depth <= MAX_DEPTH;
            }
            ~DepthGuard() noexcept { s.depth--; }
        };
    };

    NodePtr parse_expression(ParserState& state) noexcept;
    NodePtr parse_pow(ParserState& state) noexcept;

    NodePtr parse_number(ParserState& state) noexcept {
        size_t start = state.pos;
        while (state.has_more() && (::std::isdigit(static_cast<unsigned char>(state.peek())) || state.peek() == '.')) state.get();
        auto val_opt = Utils::FastParseDouble(state.input.substr(start, state.pos - start));
        double val = val_opt.value_or(0.0);
        return state.arena.template alloc<NumberNode>(val);
    }

    NodePtr parse_identifier(ParserState& state) noexcept {
        size_t start = state.pos;
        while (state.has_more() && (::std::isalnum(static_cast<unsigned char>(state.peek())) || state.peek() == '_')) state.get();
        ::std::string_view name = state.input.substr(start, state.pos - start);
        
        state.skip_ws();
        if (state.peek() == '(') {
            state.get(); // '('
            auto arg = parse_expression(state);
            if (!arg) return nullptr;
            state.match(')');
            return state.arena.template alloc<UnaryOpNode>(state.arena.allocString(name), arg);
        }
        
        if (name == "sin" || name == "cos" || name == "sqrt" || name == "tan" || name == "log" || name == "exp") {
            auto arg = parse_pow(state);
            if (!arg) return nullptr;
            return state.arena.template alloc<UnaryOpNode>(state.arena.allocString(name), arg);
        }
        
        return state.arena.template alloc<VariableNode>(state.arena.allocString(name));
    }

    NodePtr parse_matrix(ParserState& state) noexcept {
        state.get(); // '['
        Matrix m;
        state.skip_ws();
        while (state.has_more() && state.peek() != ']') {
            if (state.match('[')) {
                AXIOM::Vector row;
                while (state.has_more() && state.peek() != ']') {
                    size_t start = state.pos;
                    while (state.has_more() && (std::isdigit(static_cast<unsigned char>(state.peek())) || state.peek() == '.' || state.peek() == '-')) state.get();
                    auto val = Utils::FastParseDouble(state.input.substr(start, state.pos - start));
                    if (val) row.push_back(*val);
                    state.match(',');
                    state.skip_ws();
                }
                m.push_back(std::move(row));
                state.match(']');
            }
            state.match(',');
            state.skip_ws();
        }
        state.match(']');
        return state.arena.template alloc<MatrixNode>(std::move(m));
    }

    NodePtr parse_primary(ParserState& state) noexcept {
        ParserState::DepthGuard guard(state);
        if (!guard.ok) return nullptr;

        state.skip_ws();
        char p = state.peek();
        if (p == '(') {
            state.pos++;
            auto node = parse_expression(state);
            state.match(')');
            return node;
        }

        if (p == '[') {
            return parse_matrix(state);
        }

        if (::std::isdigit(static_cast<unsigned char>(p)) || p == '.') {
            return parse_number(state);
        }

        if (::std::isalpha(static_cast<unsigned char>(p))) {
            return parse_identifier(state);
        }

        if (p == '-') {
            state.pos++;
            auto arg = parse_primary(state);
            if (!arg) return nullptr;
            return state.arena.template alloc<UnaryOpNode>("u-", arg);
        }

        return state.arena.template alloc<NumberNode>(0.0);
    }

    NodePtr parse_pow(ParserState& state) noexcept {
        ParserState::DepthGuard guard(state);
        if (!guard.ok) return nullptr;

        auto left = parse_primary(state);
        if (!left) return nullptr;
        state.skip_ws();
        while (state.match('^')) {
            auto right = parse_pow(state);
            if (!right) return nullptr;
            left = state.arena.template alloc<BinaryOpNode>('^', left, right);
            state.skip_ws();
        }
        return left;
    }

    NodePtr parse_mul_div(ParserState& state) noexcept {
        ParserState::DepthGuard guard(state);
        if (!guard.ok) return nullptr;

        auto left = parse_pow(state);
        if (!left) return nullptr;
        state.skip_ws();
        while (state.peek() == '*' || state.peek() == '/') {
            char op = state.get();
            auto right = parse_pow(state);
            if (!right) return nullptr;
            if (left->IsMatrix() || right->IsMatrix()) {
                left = state.arena.template alloc<MatrixBinaryOpNode>(op, left, right);
            } else {
                left = state.arena.template alloc<BinaryOpNode>(op, left, right);
            }
            state.skip_ws();
        }
        return left;
    }

    NodePtr parse_expression(ParserState& state) noexcept {
        ParserState::DepthGuard guard(state);
        if (!guard.ok) return nullptr;

        auto left = parse_mul_div(state);
        if (!left) return nullptr;
        state.skip_ws();
        while (state.peek() == '+' || state.peek() == '-') {
            char op = state.get();
            auto right = parse_mul_div(state);
            if (!right) return nullptr;
            if (left->IsMatrix() || right->IsMatrix()) {
                left = state.arena.template alloc<MatrixBinaryOpNode>(op, left, right);
            } else {
                left = state.arena.template alloc<BinaryOpNode>(op, left, right);
            }
            state.skip_ws();
        }
        return left;
    }
}

Precedence GetOpPrecedence(char op) noexcept {
    if (op == '+' || op == '-') return Precedence::AddSub;
    if (op == '*' || op == '/') return Precedence::MultiDiv;
    if (op == '^') return Precedence::Pow;
    return Precedence::None;
}

bool IsConst(const NodePtr node, double val) noexcept {
    auto res = node->Evaluate({});
    if (!res.HasValue()) return false;
    double node_val = GetReal(*res.value);
    return ::std::abs(node_val - val) < 1e-9;
}

CalcErr NormalizeError(const EvalResult& res, CalcErr fallback = CalcErr::ArgumentMismatch) noexcept {
    return res.error == CalcErr::None ? fallback : res.error;
}

::std::string_view FormatNumber(Arena& arena, double val) noexcept {
    char buf[64];
    int len = 0;
    if (::std::isinf(val)) len = snprintf(buf, sizeof(buf), ::std::signbit(val) ? "-inf" : "inf");
    else if (::std::isnan(val)) len = snprintf(buf, sizeof(buf), "nan");
    else if (val == ::std::floor(val) && ::std::abs(val) < 1e15) len = snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(val));
    else {
        const double abs_val = ::std::abs(val);
        if (abs_val >= 1e6 || (abs_val > 0 && abs_val < 1e-6)) len = snprintf(buf, sizeof(buf), "%.6e", val);
        else len = snprintf(buf, sizeof(buf), "%.15g", val);
    }
    return arena.allocString(std::string_view(buf, len));
}

// MatrixNode Implementations
EvalResult MatrixNode::Evaluate(const SymbolTable& vars) const noexcept {
    return EvalResult::Success(matrix);
}

#if defined(ASMJIT_BUILD_X86)
bool MatrixNode::CompileMatrixX86(::asmjit::x86::Compiler& cc, ::asmjit::x86::Gp vars_ptr, const SymbolTable& var_map, FixedVector<::asmjit::x86::Vec, 256>& out) const noexcept {
    auto [rows, cols] = GetMatrixDims();
    if (rows != 4 || cols != 4) return false;
    out.clear();
    for (int i = 0; i < 4; ++i) {
        ::asmjit::x86::Vec row_v = cc.new_ymm();
        cc.vmovupd(row_v, cc.new_const(::asmjit::ConstPoolScope::kLocal, &matrix[i][0], 32));
        out.push_back(row_v);
    }
    return true;
}
#endif

#if defined(ASMJIT_BUILD_AARCH64)
bool MatrixNode::CompileMatrixAArch64(::asmjit::aarch64::Compiler& cc, ::asmjit::aarch64::Gp vars_ptr, const SymbolTable& var_map, FixedVector<::asmjit::aarch64::Vec, 256>& out) const noexcept {
    auto [rows, cols] = GetMatrixDims();
    if (rows != 4 || cols != 4) return false;
    out.clear();
    for (int i = 0; i < 4; ++i) {
        ::asmjit::aarch64::Vec row_v = cc.new_v("row");
        cc.ld1(row_v.v4d(), ::asmjit::aarch64::ptr(cc.new_const(::asmjit::ConstPoolScope::kLocal, &matrix[i][0], 32)));
        out.push_back(row_v);
    }
    return true;
}
#endif

void MatrixNode::CollectVariables(SymbolTable& var_map) const noexcept {}

std::string_view MatrixNode::ToString(Arena& arena, Precedence) const noexcept {
    char buf[4096];
    int pos = 0;
    pos += snprintf(buf + pos, sizeof(buf) - pos, "[");
    for (size_t i = 0; i < matrix.size(); ++i) {
        pos += snprintf(buf + pos, sizeof(buf) - pos, "[");
        for (size_t j = 0; j < matrix[i].size(); ++j) {
            std::string_view num_sv = FormatNumber(arena, matrix[i][j]);
            pos += snprintf(buf + pos, sizeof(buf) - pos, "%.*s", (int)num_sv.length(), num_sv.data());
            if (j < matrix[i].size() - 1) pos += snprintf(buf + pos, sizeof(buf) - pos, ", ");
        }
        pos += snprintf(buf + pos, sizeof(buf) - pos, "]");
        if (i < matrix.size() - 1) pos += snprintf(buf + pos, sizeof(buf) - pos, ", ");
    }
    pos += snprintf(buf + pos, sizeof(buf) - pos, "]");
    return arena.allocString(std::string_view(buf, pos));
}

// MatrixBinaryOpNode Implementations
EvalResult MatrixBinaryOpNode::Evaluate(const SymbolTable& vars) const noexcept {
    auto l_res = left->Evaluate(vars); if (!l_res.HasValue()) return l_res;
    auto r_res = right->Evaluate(vars); if (!r_res.HasValue()) return r_res;
    if (!l_res.matrix.has_value() || !r_res.matrix.has_value()) return EvalResult::Failure(CalcErr::ArgumentMismatch);
    const auto& A = *l_res.matrix; const auto& B = *r_res.matrix;
    if (op == '*') {
        if (A.empty() || B.empty() || A[0].size() != B.size()) return EvalResult::Failure(CalcErr::ArgumentMismatch);
        Matrix C;
        for (size_t i = 0; i < A.size(); ++i) {
            Vector row;
            for (size_t j = 0; j < B[0].size(); ++j) {
                double sum = 0;
                for (size_t k = 0; k < B.size(); ++k) sum += A[i][k] * B[k][j];
                row.push_back(sum);
            }
            C.push_back(std::move(row));
        }
        return EvalResult::Success(std::move(C));
    }
    if (op == '+' || op == '-') {
        if (A.size() != B.size() || A[0].size() != B[0].size()) return EvalResult::Failure(CalcErr::ArgumentMismatch);
        Matrix C;
        for (size_t i = 0; i < A.size(); ++i) {
            Vector row;
            for (size_t j = 0; j < A[i].size(); ++j) row.push_back(op == '+' ? A[i][j] + B[i][j] : A[i][j] - B[i][j]);
            C.push_back(std::move(row));
        }
        return EvalResult::Success(std::move(C));
    }
    return EvalResult::Failure(CalcErr::OperationNotFound);
}

#if defined(ASMJIT_BUILD_X86)
bool MatrixBinaryOpNode::CompileMatrixX86(::asmjit::x86::Compiler& cc, ::asmjit::x86::Gp vars_ptr, const SymbolTable& var_map, FixedVector<::asmjit::x86::Vec, 256>& out) const noexcept {
    FixedVector<::asmjit::x86::Vec, 256> l_v, r_v;
    if (!left->CompileMatrixX86(cc, vars_ptr, var_map, l_v) || !right->CompileMatrixX86(cc, vars_ptr, var_map, r_v)) return false;
    auto [rows, cols] = GetMatrixDims();
    if (rows != 4 || cols != 4) return false;
    if (op == '*') {
        out.clear();
        for (int i = 0; i < 4; ++i) {
            ::asmjit::x86::Vec row_res = cc.new_ymm();
            for (int k = 0; k < 4; ++k) {
                ::asmjit::x86::Vec a_ik = cc.new_ymm();
                if (left->GetType() == NodeType::Matrix) {
                    cc.vbroadcastsd(a_ik, cc.new_const(::asmjit::ConstPoolScope::kLocal, &static_cast<MatrixNode*>(left)->matrix[i][k], 8));
                } else {
                    if (k < 2) {
                        if (k == 0) cc.vbroadcastsd(a_ik, l_v[i]);
                        else { auto tmp = cc.new_xmm(); cc.vpermilpd(tmp, l_v[i], 1); cc.vbroadcastsd(a_ik, tmp); }
                    } else {
                        auto tmp = cc.new_xmm(); cc.vextractf128(tmp, l_v[i], 1);
                        if (k == 2) cc.vbroadcastsd(a_ik, tmp);
                        else { cc.vpermilpd(tmp, tmp, 1); cc.vbroadcastsd(a_ik, tmp); }
                    }
                }
                if (k == 0) cc.vmulpd(row_res, a_ik, r_v[k]);
                else cc.vfmadd231pd(row_res, a_ik, r_v[k]);
            }
            out.push_back(row_res);
        }
        return true;
    }
    return false;
}
#endif

#if defined(ASMJIT_BUILD_AARCH64)
bool MatrixBinaryOpNode::CompileMatrixAArch64(::asmjit::aarch64::Compiler& cc, ::asmjit::aarch64::Gp vars_ptr, const SymbolTable& var_map, FixedVector<::asmjit::aarch64::Vec, 256>& out) const noexcept {
    FixedVector<::asmjit::aarch64::Vec, 256> l_v, r_v;
    if (!left->CompileMatrixAArch64(cc, vars_ptr, var_map, l_v) || !right->CompileMatrixAArch64(cc, vars_ptr, var_map, r_v)) return false;
    if (op == '*') {
        out.clear();
        for (int i = 0; i < 4; ++i) {
            ::asmjit::aarch64::Vec row_res = cc.new_v("res");
            cc.fmov(row_res.v4d(), 0.0);
            for (int k = 0; k < 4; ++k) {
                ::asmjit::aarch64::Vec a_ik = cc.new_v("aik");
                cc.dup(a_ik.v4d(), l_v[i].v2d(), k); 
                cc.fmla(row_res.v4d(), a_ik.v4d(), r_v[k].v4d());
            }
            out.push_back(row_res);
        }
        return true;
    }
    return false;
}
#endif

void MatrixBinaryOpNode::CollectVariables(SymbolTable& var_map) const noexcept {
    if (left) left->CollectVariables(var_map);
    if (right) right->CollectVariables(var_map);
}

std::string_view MatrixBinaryOpNode::ToString(Arena& arena, Precedence p) const noexcept {
    std::string_view l = left->ToString(arena, Precedence::MultiDiv);
    std::string_view r = right->ToString(arena, Precedence::MultiDiv);
    char buf[1024];
    int len = snprintf(buf, sizeof(buf), "%.*s %c %.*s", (int)l.length(), l.data(), op, (int)r.length(), r.data());
    return arena.allocString(std::string_view(buf, len));
}

// NumberNode Implementations
#if defined(ASMJIT_BUILD_X86)
bool NumberNode::CompileX86(::asmjit::x86::Compiler& cc, ::asmjit::x86::Gp vars_ptr, const SymbolTable&, ::asmjit::x86::Vec& out) const noexcept {
    out = cc.new_xmm();
    cc.movsd(out, cc.new_double_const(::asmjit::ConstPoolScope::kLocal, value));
    return true;
}
#endif

#if defined(ASMJIT_BUILD_AARCH64)
bool NumberNode::CompileAArch64(::asmjit::aarch64::Compiler& cc, ::asmjit::aarch64::Gp vars_ptr, const SymbolTable&, ::asmjit::aarch64::Vec& out) const noexcept {
    out = cc.new_v("num");
    cc.fmov(out.d(0), value);
    return true;
}
#endif

EvalResult NumberNode::Evaluate(const SymbolTable&) const noexcept { return EvalResult::Success(value); }
void NumberNode::CollectVariables(SymbolTable&) const noexcept {}
std::string_view NumberNode::ToString(Arena& arena, Precedence) const noexcept { return FormatNumber(arena, value); }
ExprNode* NumberNode::Derivative(Arena& arena, ::std::string_view) const noexcept { return arena.template alloc<NumberNode>(0.0); }
ExprNode* NumberNode::Simplify(Arena& arena) const noexcept { return (ExprNode*)this; }

EvalResult VariableNode::Evaluate(const SymbolTable& vars) const noexcept {        
    int idx = LookupSymbol(vars, name);
    if (idx != -1) return EvalResult::Success(vars[idx].second);
    if (name == "pi") return EvalResult::Success(PI_CONST);
    if (name == "e") return EvalResult::Success(::std::numbers::e);
    return EvalResult::Failure(CalcErr::OperationNotFound);
}

void VariableNode::CollectVariables(SymbolTable& var_map) const noexcept {
    if (name != "pi" && name != "e") {
        if (LookupSymbol(var_map, name) == -1) var_map.push_back({name, Number(0.0)});
    }
}

#if defined(ASMJIT_BUILD_X86)
bool VariableNode::CompileX86(::asmjit::x86::Compiler& cc, ::asmjit::x86::Gp vars_ptr, const SymbolTable& var_map, ::asmjit::x86::Vec& out) const noexcept {
    int idx = LookupSymbol(var_map, name);
    out = cc.new_xmm();
    if (idx != -1) { cc.movsd(out, ::asmjit::x86::ptr(vars_ptr, idx * sizeof(double))); return true; }
    if (name == "pi") { cc.movsd(out, cc.new_double_const(::asmjit::ConstPoolScope::kLocal, PI_CONST)); return true; }
    if (name == "e") { cc.movsd(out, cc.new_double_const(::asmjit::ConstPoolScope::kLocal, ::std::numbers::e)); return true; }
    return false;
}
#endif

#if defined(ASMJIT_BUILD_AARCH64)
bool VariableNode::CompileAArch64(::asmjit::aarch64::Compiler& cc, ::asmjit::aarch64::Gp vars_ptr, const SymbolTable& var_map, ::asmjit::aarch64::Vec& out) const noexcept {
    int idx = LookupSymbol(var_map, name);
    out = cc.new_v("var");
    if (idx != -1) { cc.ldr(out.d(0), ::asmjit::aarch64::ptr(vars_ptr, idx * sizeof(double))); return true; }
    if (name == "pi") { cc.fmov(out.d(0), PI_CONST); return true; }
    if (name == "e") { cc.fmov(out.d(0), ::std::numbers::e); return true; }
    return false;
}
#endif

std::string_view VariableNode::ToString(Arena&, Precedence) const noexcept { return name; }
ExprNode* VariableNode::Derivative(Arena& arena, ::std::string_view var) const noexcept { return (name == var) ? arena.template alloc<NumberNode>(1.0) : arena.template alloc<NumberNode>(0.0); }
ExprNode* VariableNode::Simplify(Arena& arena) const noexcept { return (ExprNode*)this; }

EvalResult BinaryOpNode::Evaluate(const SymbolTable& vars) const noexcept {        
    auto l_res = left->Evaluate(vars); if (!l_res.HasValue()) return l_res;
    auto r_res = right->Evaluate(vars); if (!r_res.HasValue()) return r_res;
    double l = GetReal(*l_res.value), r = GetReal(*r_res.value);
    switch(op) {
        case '+': return EvalResult::Success(l + r);
        case '-': return EvalResult::Success(l - r);
        case '*': return EvalResult::Success(l * r);
        case '/': return (r == 0) ? EvalResult::Failure(CalcErr::DivideByZero) : EvalResult::Success(l / r);
        case '^': return EvalResult::Success(::std::pow(l, r));
        default: return EvalResult::Failure(CalcErr::OperationNotFound);
    }
}

#if defined(ASMJIT_BUILD_X86)
bool BinaryOpNode::CompileX86(::asmjit::x86::Compiler& cc, ::asmjit::x86::Gp vars_ptr, const SymbolTable& var_map, ::asmjit::x86::Vec& out) const noexcept {
    ::asmjit::x86::Vec l_v, r_v;
    if (!left->CompileX86(cc, vars_ptr, var_map, l_v) || !right->CompileX86(cc, vars_ptr, var_map, r_v)) return false;
    out = cc.new_xmm(); cc.movsd(out, l_v);
    switch(op) {
        case '+': cc.addsd(out, r_v); break;
        case '-': cc.subsd(out, r_v); break;
        case '*': cc.mulsd(out, r_v); break;
        case '/': cc.divsd(out, r_v); break;
        case '^': {
            ::asmjit::InvokeNode* invoke;
            cc.invoke(::asmjit::Out<::asmjit::InvokeNode*>(invoke), ::asmjit::Imm((intptr_t)jit_pow), ::asmjit::FuncSignature::build<double, double, double>(::asmjit::CallConvId::kCDecl));
            invoke->set_arg(0, l_v); invoke->set_arg(1, r_v); invoke->set_ret(0, out);
            break;
        }
        default: return false;
    }
    return true;
}
#endif

#if defined(ASMJIT_BUILD_AARCH64)
bool BinaryOpNode::CompileAArch64(::asmjit::aarch64::Compiler& cc, ::asmjit::aarch64::Gp vars_ptr, const SymbolTable& var_map, ::asmjit::aarch64::Vec& out) const noexcept {
    ::asmjit::aarch64::Vec l_v, r_v;
    if (!left->CompileAArch64(cc, vars_ptr, var_map, l_v) || !right->CompileAArch64(cc, vars_ptr, var_map, r_v)) return false;
    out = cc.new_v("bin_res");
    switch(op) {
        case '+': cc.fadd(out.d(0), l_v.d(0), r_v.d(0)); break;
        case '-': cc.fsub(out.d(0), l_v.d(0), r_v.d(0)); break;
        case '*': cc.fmul(out.d(0), l_v.d(0), r_v.d(0)); break;
        case '/': cc.fdiv(out.d(0), l_v.d(0), r_v.d(0)); break;
        case '^': {
            ::asmjit::InvokeNode* invoke;
            cc.invoke(::asmjit::Out<::asmjit::InvokeNode*>(invoke), ::asmjit::Imm((intptr_t)jit_pow), ::asmjit::FuncSignature::build<double, double, double>(::asmjit::CallConvId::kCDecl));
            invoke->set_arg(0, l_v.d(0)); invoke->set_arg(1, r_v.d(0)); invoke->set_ret(0, out.d(0));
            break;
        }
        default: return false;
    }
    return true;
}
#endif

void BinaryOpNode::CollectVariables(SymbolTable& var_map) const noexcept {
    if (left) left->CollectVariables(var_map);
    if (right) right->CollectVariables(var_map);
}

std::string_view BinaryOpNode::ToString(Arena& arena, Precedence p) const noexcept {
    Precedence my = GetOpPrecedence(op);
    std::string_view l = left->ToString(arena, my); std::string_view r = right->ToString(arena, my);
    char buf[2048]; int len = 0;
    if (static_cast<int>(my) < static_cast<int>(p)) len = snprintf(buf, sizeof(buf), "(%.*s %c %.*s)", (int)l.length(), l.data(), op, (int)r.length(), r.data());
    else len = snprintf(buf, sizeof(buf), "%.*s %c %.*s", (int)l.length(), l.data(), op, (int)r.length(), r.data());
    return arena.allocString(std::string_view(buf, len));
}

ExprNode* BinaryOpNode::Derivative(Arena& arena, ::std::string_view var) const noexcept {
    auto dl = left->Derivative(arena, var); auto dr = right->Derivative(arena, var);
    if (op == '+' || op == '-') return arena.template alloc<BinaryOpNode>(op, dl, dr);
    if (op == '*') return arena.template alloc<BinaryOpNode>('+', arena.template alloc<BinaryOpNode>('*', dl, right), arena.template alloc<BinaryOpNode>('*', left, dr));
    if (op == '/') {
        auto num = arena.template alloc<BinaryOpNode>('-', arena.template alloc<BinaryOpNode>('*', dl, right), arena.template alloc<BinaryOpNode>('*', left, dr));
        auto den = arena.template alloc<BinaryOpNode>('^', right, arena.template alloc<NumberNode>(2.0));
        return arena.template alloc<BinaryOpNode>('/', num, den);
    }
    return arena.template alloc<NumberNode>(0.0);
}

ExprNode* BinaryOpNode::Simplify(Arena& arena) const noexcept {
    auto sl = left->Simplify(arena); auto sr = right->Simplify(arena);
    return arena.template alloc<BinaryOpNode>(op, sl, sr);
}

EvalResult UnaryOpNode::Evaluate(const SymbolTable& vars) const noexcept {
    auto res = operand->Evaluate(vars); if (!res.HasValue()) return res;
    double v = GetReal(*res.value);
    if (func == "sin") return EvalResult::Success(::std::sin(v * D2R));
    if (func == "cos") return EvalResult::Success(::std::cos(v * D2R));
    if (func == "tan") return EvalResult::Success(::std::tan(v * D2R));
    if (func == "asin") return EvalResult::Success(::std::asin(v) * R2D);
    if (func == "acos") return EvalResult::Success(::std::acos(v) * R2D);
    if (func == "atan") return EvalResult::Success(::std::atan(v) * R2D);
    if (func == "sqrt") return (v < 0) ? EvalResult::Failure(CalcErr::NegativeRoot) : EvalResult::Success(::std::sqrt(v));
    if (func == "abs") return EvalResult::Success(::std::abs(v));
    if (func == "log") return (v <= 0) ? EvalResult::Failure(CalcErr::DomainError) : EvalResult::Success(::std::log10(v));
    if (func == "ln") return (v <= 0) ? EvalResult::Failure(CalcErr::DomainError) : EvalResult::Success(::std::log(v));
    if (func == "exp") return EvalResult::Success(::std::exp(v));
    if (func == "u-") return EvalResult::Success(-v);
    return EvalResult::Failure(CalcErr::OperationNotFound);
}

#if defined(ASMJIT_BUILD_X86)
bool UnaryOpNode::CompileX86(::asmjit::x86::Compiler& cc, ::asmjit::x86::Gp vars_ptr, const SymbolTable& var_map, ::asmjit::x86::Vec& out) const noexcept {
    ::asmjit::x86::Vec arg_v;
    if (!operand->CompileX86(cc, vars_ptr, var_map, arg_v)) return false;
    out = cc.new_xmm();
    if (func == "u-") { ::asmjit::x86::Vec zero = cc.new_xmm(); cc.xorps(zero, zero); cc.movsd(out, zero); cc.subsd(out, arg_v); return true; }
    auto call_math_func = [&](auto func_ptr) {
        ::asmjit::InvokeNode* invoke;
        cc.invoke(::asmjit::Out<::asmjit::InvokeNode*>(invoke), ::asmjit::Imm((intptr_t)func_ptr), ::asmjit::FuncSignature::build<double, double>(::asmjit::CallConvId::kCDecl));
        invoke->set_arg(0, arg_v); invoke->set_ret(0, out);
    };
    if (func == "abs") { call_math_func(jit_abs); return true; }
    if (func == "sqrt") { cc.sqrtsd(out, arg_v); return true; }
    if (func == "exp") { call_math_func(jit_exp); return true; }
    if (func == "log") { call_math_func(jit_log10); return true; }
    if (func == "ln") { call_math_func(jit_log); return true; }
    auto convert_to_rad = [&]() { ::asmjit::x86::Vec rad_v = cc.new_xmm(); cc.movsd(rad_v, cc.new_double_const(::asmjit::ConstPoolScope::kLocal, D2R)); cc.mulsd(rad_v, arg_v); return rad_v; };
    if (func == "sin") { ::asmjit::x86::Vec rad = convert_to_rad(); call_math_func(jit_sin); return true; }
    if (func == "cos") { ::asmjit::x86::Vec rad = convert_to_rad(); call_math_func(jit_cos); return true; }
    if (func == "tan") { ::asmjit::x86::Vec rad = convert_to_rad(); call_math_func(jit_tan); return true; }
    return false;
}

bool UnaryOpNode::CompileMatrixX86(::asmjit::x86::Compiler& cc, ::asmjit::x86::Gp vars_ptr, const SymbolTable& var_map, FixedVector<::asmjit::x86::Vec, 256>& out) const noexcept {
    FixedVector<::asmjit::x86::Vec, 256> arg_v;
    if (!operand->CompileMatrixX86(cc, vars_ptr, var_map, arg_v)) return false;
    auto call_sleef_simd = [&](void* sleef_func) {
        for (auto& vec : arg_v) {
            ::asmjit::x86::Vec res = cc.new_ymm(); ::asmjit::InvokeNode* invoke;
            ::asmjit::FuncSignature sig(::asmjit::CallConvId::kCDecl, 0, ::asmjit::TypeId::kFloat64x4); sig.add_arg(::asmjit::TypeId::kFloat64x4);
            cc.invoke(::asmjit::Out<::asmjit::InvokeNode*>(invoke), ::asmjit::Imm((intptr_t)sleef_func), sig);
            invoke->set_arg(0, vec); invoke->set_ret(0, res); out.push_back(res);
        }
    };
    if (func == "sin") { call_sleef_simd((void*)Sleef_sind4_u10avx2); return true; }
    if (func == "cos") { call_sleef_simd((void*)Sleef_cosd4_u10avx2); return true; }
    if (func == "tan") { call_sleef_simd((void*)Sleef_tand4_u10avx2); return true; }
    if (func == "log") { call_sleef_simd((void*)Sleef_log10d4_u10avx2); return true; }
    if (func == "ln")  { call_sleef_simd((void*)Sleef_logd4_u10avx2); return true; }
    if (func == "exp") { call_sleef_simd((void*)Sleef_expd4_u10avx2); return true; }
    if (func == "abs") {
        for (auto& vec : arg_v) {
            ::asmjit::x86::Vec res = cc.new_ymm(); ::asmjit::x86::Vec mask = cc.new_ymm();
            cc.vmovq(mask.xmm(), cc.new_int64_const(::asmjit::ConstPoolScope::kLocal, 0x7FFFFFFFFFFFFFFF));
            cc.vpbroadcastq(mask, mask.xmm()); cc.vpand(res, vec, mask); out.push_back(res);
        }
        return true;
    }
    if (func == "sqrt") { for (auto& vec : arg_v) { ::asmjit::x86::Vec res = cc.new_ymm(); cc.vsqrtpd(res, vec); out.push_back(res); } return true; }
    return false;
}
#endif

#if defined(ASMJIT_BUILD_AARCH64)
bool UnaryOpNode::CompileAArch64(::asmjit::aarch64::Compiler& cc, ::asmjit::aarch64::Gp vars_ptr, const SymbolTable& var_map, ::asmjit::aarch64::Vec& out) const noexcept {
    ::asmjit::aarch64::Vec arg_v;
    if (!operand->CompileAArch64(cc, vars_ptr, var_map, arg_v)) return false;
    out = cc.new_v("un_res");
    if (func == "u-") { cc.fneg(out.d(0), arg_v.d(0)); return true; }
    auto call_math_func = [&](auto func_ptr) {
        ::asmjit::InvokeNode* invoke;
        cc.invoke(::asmjit::Out<::asmjit::InvokeNode*>(invoke), ::asmjit::Imm((intptr_t)func_ptr), ::asmjit::FuncSignature::build<double, double>(::asmjit::CallConvId::kCDecl));
        invoke->set_arg(0, arg_v.d(0)); invoke->set_ret(0, out.d(0));
    };
    if (func == "abs") { cc.fabs(out.d(0), arg_v.d(0)); return true; }
    if (func == "sqrt") { cc.fsqrt(out.d(0), arg_v.d(0)); return true; }
    if (func == "exp") { call_math_func(jit_exp); return true; }
    if (func == "log") { call_math_func(jit_log10); return true; }
    if (func == "ln") { call_math_func(jit_log); return true; }
    auto convert_to_rad = [&]() { ::asmjit::aarch64::Vec rad_v = cc.new_v("rad"); cc.fmov(rad_v.d(0), D2R); cc.fmul(rad_v.d(0), rad_v.d(0), arg_v.d(0)); return rad_v; };
    if (func == "sin") { ::asmjit::aarch64::Vec rad = convert_to_rad(); call_math_func(jit_sin); return true; }
    return false;
}

bool UnaryOpNode::CompileMatrixAArch64(::asmjit::aarch64::Compiler& cc, ::asmjit::aarch64::Gp vars_ptr, const SymbolTable& var_map, FixedVector<::asmjit::aarch64::Vec, 256>& out) const noexcept {
    FixedVector<::asmjit::aarch64::Vec, 256> arg_v;
    if (!operand->CompileMatrixAArch64(cc, vars_ptr, var_map, arg_v)) return false;
    if (func == "u-") { for (auto& vec : arg_v) { ::asmjit::aarch64::Vec res = cc.new_v("res"); cc.fneg(res.v4d(), vec.v4d()); out.push_back(res); } return true; }
    if (func == "abs") { for (auto& vec : arg_v) { ::asmjit::aarch64::Vec res = cc.new_v("res"); cc.fabs(res.v4d(), vec.v4d()); out.push_back(res); } return true; }
    if (func == "sqrt") { for (auto& vec : arg_v) { ::asmjit::aarch64::Vec res = cc.new_v("res"); cc.fsqrt(res.v4d(), vec.v4d()); out.push_back(res); } return true; }
    return false;
}
#endif

void UnaryOpNode::CollectVariables(SymbolTable& var_map) const noexcept { if (operand) operand->CollectVariables(var_map); }
std::string_view UnaryOpNode::ToString(Arena& arena, Precedence) const noexcept {
    std::string_view arg = operand->ToString(arena);
    char buf[1024]; int len = snprintf(buf, sizeof(buf), "%.*s(%.*s)", (int)func.length(), func.data(), (int)arg.length(), arg.data());
    return arena.allocString(std::string_view(buf, len));
}
ExprNode* UnaryOpNode::Derivative(Arena& arena, ::std::string_view var) const noexcept { return arena.template alloc<NumberNode>(0.0); }
ExprNode* UnaryOpNode::Simplify(Arena& arena) const noexcept { return arena.template alloc<UnaryOpNode>(func, operand->Simplify(arena)); }

EvalResult MultiArgFunctionNode::Evaluate(const SymbolTable& vars) const noexcept { return EvalResult::Failure(CalcErr::OperationNotFound); }
#if defined(ASMJIT_BUILD_X86)
bool MultiArgFunctionNode::CompileX86(::asmjit::x86::Compiler&, ::asmjit::x86::Gp, const SymbolTable&, ::asmjit::x86::Vec&) const noexcept { return false; }
#endif
#if defined(ASMJIT_BUILD_AARCH64)
bool MultiArgFunctionNode::CompileAArch64(::asmjit::aarch64::Compiler&, ::asmjit::aarch64::Gp, const SymbolTable&, ::asmjit::aarch64::Vec&) const noexcept { return false; }
#endif

void MultiArgFunctionNode::CollectVariables(SymbolTable& var_map) const noexcept { for (auto arg : args) if (arg) arg->CollectVariables(var_map); }
std::string_view MultiArgFunctionNode::ToString(Arena& arena, Precedence) const noexcept {
    char buf[256]; int len = snprintf(buf, sizeof(buf), "%.*s(...)", (int)func.length(), func.data());
    return arena.allocString(std::string_view(buf, len));
}
ExprNode* MultiArgFunctionNode::Derivative(Arena& a, ::std::string_view v) const noexcept { return a.template alloc<NumberNode>(0.0); }
ExprNode* MultiArgFunctionNode::Simplify(Arena& a) const noexcept { return (ExprNode*)this; }

AlgebraicParser::AlgebraicParser() {}
void AlgebraicParser::RegisterSpecialCommands() {}
NodePtr AlgebraicParser::ParseExpression(::std::string_view input) { ParserState state{input, 0, arena_}; return parse_expression(state); }
EngineResult AlgebraicParser::ParseAndExecute(std::string_view input) { 
    ::std::string_view trimmed = Utils::TrimSV(input);
    if (trimmed.rfind("derive ", 0) == 0) return HandleDerivative(trimmed);
    if (trimmed.rfind("limit(", 0) == 0) return HandleLimit(trimmed);
    if (trimmed.rfind("integrate(", 0) == 0) return HandleIntegrate(trimmed);
    return ParseAndExecuteWithContext(trimmed, SymbolTable{}); 
}

EngineResult AlgebraicParser::HandleLimit(std::string_view input) {
    auto start = input.find('('); auto end = input.rfind(')');
    if (start == ::std::string_view::npos || end == ::std::string_view::npos || end <= start) return CreateErrorResult(CalcErr::ParseError);
    std::string_view content = input.substr(start + 1, end - start - 1);
    size_t c1 = content.find(','); if (c1 == std::string_view::npos) return CreateErrorResult(CalcErr::ArgumentMismatch);
    std::string_view expr = Utils::TrimSV(content.substr(0, c1));
    std::string_view rest = content.substr(c1 + 1);
    size_t c2 = rest.find(','); if (c2 == std::string_view::npos) return CreateErrorResult(CalcErr::ArgumentMismatch);
    std::string_view var = Utils::TrimSV(rest.substr(0, c2));
    std::string_view point_str = Utils::TrimSV(rest.substr(c2 + 1));
    auto point_val = Utils::FastParseDouble(point_str); if (!point_val) return CreateErrorResult(CalcErr::ParseError);
    SymbolTable ctx; ctx.push_back({var, Number(*point_val)});
    return ParseAndExecuteWithContext(expr, ctx);
}

EngineResult AlgebraicParser::HandleIntegrate(std::string_view input) {
    auto start = input.find('('); auto end = input.rfind(')');
    if (start == ::std::string_view::npos || end == ::std::string_view::npos || end <= start) return CreateErrorResult(CalcErr::ParseError);
    std::string_view content = input.substr(start + 1, end - start - 1);
    size_t c1 = content.find(','); if (c1 == std::string_view::npos) return CreateErrorResult(CalcErr::ArgumentMismatch);
    std::string_view expr = Utils::TrimSV(content.substr(0, c1));
    std::string_view rest1 = content.substr(c1 + 1);
    size_t c2 = rest1.find(','); if (c2 == std::string_view::npos) return CreateErrorResult(CalcErr::ArgumentMismatch);
    std::string_view var = Utils::TrimSV(rest1.substr(0, c2));
    std::string_view rest2 = rest1.substr(c2 + 1);
    size_t c3 = rest2.find(','); if (c3 == std::string_view::npos) return CreateErrorResult(CalcErr::ArgumentMismatch);
    std::string_view a_str = Utils::TrimSV(rest2.substr(0, c3));
    std::string_view b_str = Utils::TrimSV(rest2.substr(c3 + 1));
    auto a_val = Utils::FastParseDouble(a_str); auto b_val = Utils::FastParseDouble(b_str);
    if (!a_val || !b_val) return CreateErrorResult(CalcErr::ParseError);
    double a = *a_val, b = *b_val; int n = 1000; double h = (b - a) / n; double sum = 0;
    SymbolTable ctx; ctx.push_back({var, 0.0});
    auto eval = [&](double x) { ctx[0].second = Number(x); auto res = ParseAndExecuteWithContext(expr, ctx); return res.HasResult() ? res.GetDouble().value_or(0.0) : 0.0; };
    sum += 0.5 * (eval(a) + eval(b));
    for (int i = 1; i < n; i++) sum += eval(a + i * h);
    return CreateSuccessResult(sum * h);
}

EngineResult AlgebraicParser::ParseAndExecuteWithContext(std::string_view input, const SymbolTable& context) {
    auto root = ParseExpression(input); if (!root) return CreateErrorResult(CalcErr::StackOverflow);
    auto res = root->Evaluate(context); if (res.HasValue()) {
        if (res.matrix.has_value()) return CreateSuccessResult(std::move(*res.matrix));
        if (res.vector.has_value()) return CreateSuccessResult(std::move(*res.vector));
        return CreateSuccessResult(*res.value);
    }
    return CreateErrorResult(res.error);
}

EngineResult AlgebraicParser::HandleQuadratic(std::string_view input) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult AlgebraicParser::HandleNonLinearSolve(std::string_view input) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult AlgebraicParser::HandleDerivative(std::string_view input) {
    std::string_view target = input; if (target.rfind("derive ", 0) == 0) target = target.substr(7);
    auto root = ParseExpression(target); if (!root) return CreateErrorResult(CalcErr::ParseError);
    auto deriv = root->Derivative(arena_, "x"); if (!deriv) return CreateErrorResult(CalcErr::OperationNotFound);
    auto simplified = deriv->Simplify(arena_); return CreateSuccessResult(simplified->ToString(arena_));
}
EngineResult AlgebraicParser::SolveQuadratic(double a, double b, double c) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult AlgebraicParser::SolveNonLinearSystem(const FixedVector<std::string_view, 256>& equations, SymbolTable& guess) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult AlgebraicParser::HandlePlotFunction(std::string_view input) { return CreateErrorResult(CalcErr::OperationNotFound); }

} // namespace AXIOM
