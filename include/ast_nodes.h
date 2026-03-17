#pragma once

#include "dynamic_calc_types.h"
#include "arena.h"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>
#include <complex>
#include <asmjit/x86.h>

namespace AXIOM {

enum class NodeType {
    Number,
    Variable,
    BinaryOp,
    UnaryOp,
    MultiArgFunction
};

struct EvalResult {
    std::optional<Number> value;
    CalcErr error = CalcErr::None;

    static EvalResult Success(double val) {
        EvalResult result;
        result.value = Number(val);
        return result;
    }
    static EvalResult Success(const std::complex<double>& val) {
        EvalResult result;
        result.value = Number(val);
        return result;
    }
    static EvalResult Success(const Number& val) {
        EvalResult result;
        result.value = val;
        return result;
    }
    static EvalResult Failure(CalcErr err) {
        EvalResult result;
        result.error = err;
        return result;
    }
    bool HasValue() const { return value.has_value() && error == CalcErr::None; }
};

struct ExprNode;
using NodePtr = ExprNode*;

struct ExprNode {
    virtual ~ExprNode() = default;
    virtual NodeType GetType() const = 0;
    virtual EvalResult Evaluate(const std::unordered_map<std::string, Number>& vars) const = 0;
    virtual std::string ToString(Precedence parent_prec = Precedence::None) const = 0;
    virtual bool Compile(::asmjit::x86::Compiler& cc,
                         ::asmjit::x86::Gp vars_ptr,
                         const std::unordered_map<std::string, int>& var_map,
                         ::asmjit::x86::Vec& out) const = 0;
    virtual ExprNode* Derivative(Arena& arena, std::string_view var) const = 0;
    virtual ExprNode* Simplify(Arena& arena) const = 0;
};

struct NumberNode : ExprNode {
    double value;
    explicit NumberNode(double v) : value(v) {}
    NodeType GetType() const override { return NodeType::Number; }
    EvalResult Evaluate(const std::unordered_map<std::string, Number>&) const override;
    bool Compile(asmjit::x86::Compiler& cc, asmjit::x86::Gp vars_ptr, const std::unordered_map<std::string, int>&, asmjit::x86::Vec& out) const override;
    std::string ToString(Precedence) const override;
    ExprNode* Derivative(Arena& arena, std::string_view) const override;
    ExprNode* Simplify(Arena& arena) const override;
};

struct VariableNode : ExprNode {
    std::string_view name;
    explicit VariableNode(std::string_view n) : name(n) {}
    NodeType GetType() const override { return NodeType::Variable; }
    EvalResult Evaluate(const std::unordered_map<std::string, Number>& vars) const override;
    bool Compile(asmjit::x86::Compiler& cc, asmjit::x86::Gp vars_ptr, const std::unordered_map<std::string, int>& var_map, asmjit::x86::Vec& out) const override;
    std::string ToString(Precedence) const override;
    ExprNode* Derivative(Arena& arena, std::string_view var) const override;
    ExprNode* Simplify(Arena& arena) const override;
};

struct BinaryOpNode : ExprNode {
    char op; NodePtr left, right;
    BinaryOpNode(char c, NodePtr l, NodePtr r) : op(c), left(l), right(r) {}
    NodeType GetType() const override { return NodeType::BinaryOp; }
    EvalResult Evaluate(const std::unordered_map<std::string, Number>& vars) const override;
    bool Compile(asmjit::x86::Compiler& cc, asmjit::x86::Gp vars_ptr, const std::unordered_map<std::string, int>& var_map, asmjit::x86::Vec& out) const override;
    std::string ToString(Precedence p) const override;
    ExprNode* Derivative(Arena& arena, std::string_view var) const override;
    ExprNode* Simplify(Arena& arena) const override;
};

struct UnaryOpNode : ExprNode {
    std::string_view func; NodePtr operand;
    UnaryOpNode(std::string_view f, NodePtr op) : func(f), operand(op) {}
    NodeType GetType() const override { return NodeType::UnaryOp; }
    EvalResult Evaluate(const std::unordered_map<std::string, Number>& vars) const override;
    bool Compile(asmjit::x86::Compiler& cc, asmjit::x86::Gp vars_ptr, const std::unordered_map<std::string, int>& var_map, asmjit::x86::Vec& out) const override;
    std::string ToString(Precedence) const override;
    ExprNode* Derivative(Arena& arena, std::string_view var) const override;
    ExprNode* Simplify(Arena& arena) const override;
};

struct MultiArgFunctionNode : ExprNode {
    std::string_view func; std::vector<NodePtr> args;
    MultiArgFunctionNode(std::string_view f, std::vector<NodePtr> a) : func(f), args(std::move(a)) {}
    NodeType GetType() const override { return NodeType::MultiArgFunction; }
    EvalResult Evaluate(const std::unordered_map<std::string, Number>& vars) const override;
    bool Compile(asmjit::x86::Compiler& cc, asmjit::x86::Gp vars_ptr, const std::unordered_map<std::string, int>& var_map, asmjit::x86::Vec& out) const override;
    std::string ToString(Precedence) const override;
    ExprNode* Derivative(Arena& arena, std::string_view var) const override;
    ExprNode* Simplify(Arena& arena) const override;
};

} // namespace AXIOM

