// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once

#include "dynamic_calc_types.h"
#include "arena.h"
#include "fixed_vector.h"
#include <string_view>
#include <variant>
#include <optional>
#include <complex>
#include <asmjit/core.h>

#if defined(ASMJIT_BUILD_X86)
#include <asmjit/x86.h>
#endif
#if defined(ASMJIT_BUILD_AARCH64) || defined(ASMJIT_BUILD_ARM)
#include <asmjit/arm.h>
#endif

namespace AXIOM {

using SymbolTable = FixedVector<std::pair<std::string_view, Number>, 128>;

// Forward declarations
struct NumberNode;
struct VariableNode;
struct BinaryOpNode;
struct UnaryOpNode;
struct MultiArgFunctionNode;
struct MatrixNode;
struct MatrixBinaryOpNode;

/**
 * @brief AnyNode: The master variant for Operation VARIANT SHIFT.
 * Replaces the virtual ExprNode hierarchy with zero-overhead static dispatch.
 */
using AnyNode = std::variant<
    NumberNode,
    VariableNode,
    BinaryOpNode,
    UnaryOpNode,
    MultiArgFunctionNode,
    MatrixNode,
    MatrixBinaryOpNode
>;

using NodePtr = AnyNode*;

struct EvalResult {
    std::optional<Number> value;
    std::optional<Vector> vector;
    std::optional<Matrix> matrix;
    CalcErr error = CalcErr::None;

    static EvalResult Success(double val) noexcept { EvalResult r; r.value = Number(val); return r; }
    static EvalResult Success(const Number& val) noexcept { EvalResult r; r.value = val; return r; }
    static EvalResult Success(Vector&& val) noexcept { EvalResult r; r.vector = std::move(val); return r; }
    static EvalResult Success(Matrix&& val) noexcept { EvalResult r; r.matrix = std::move(val); return r; }
    static EvalResult Failure(CalcErr err) noexcept { EvalResult r; r.error = err; return r; }
    bool HasValue() const noexcept { return (value || vector || matrix) && error == CalcErr::None; }
};

// --- Node Structs (Non-virtual, Data-oriented) ---

struct NumberNode {
    double value;
    explicit NumberNode(double v) noexcept : value(v) {}
};

struct VariableNode {
    std::string_view name;
    explicit VariableNode(std::string_view n) noexcept : name(n) {}
};

struct BinaryOpNode {
    char op;
    NodePtr left;
    NodePtr right;
    BinaryOpNode(char o, NodePtr l, NodePtr r) noexcept : op(o), left(l), right(r) {}
};

struct UnaryOpNode {
    std::string_view func;
    NodePtr operand;
    UnaryOpNode(std::string_view f, NodePtr op) noexcept : func(f), operand(op) {}
};

struct MultiArgFunctionNode {
    std::string_view func;
    FixedVector<NodePtr, 16> args;
    MultiArgFunctionNode(std::string_view f, FixedVector<NodePtr, 16> a) noexcept : func(f), args(std::move(a)) {}
};

struct MatrixNode {
    Matrix matrix;
    explicit MatrixNode(Matrix m) noexcept : matrix(std::move(m)) {}
};

struct MatrixBinaryOpNode {
    char op;
    NodePtr left;
    NodePtr right;
    MatrixBinaryOpNode(char o, NodePtr l, NodePtr r) noexcept : op(o), left(l), right(r) {}
};

// --- Static Dispatcher (The Core of VARIANT SHIFT) ---

struct NodeDispatcher {
    static EvalResult Evaluate(NodePtr node, const SymbolTable& vars) noexcept;
    static std::string_view ToString(NodePtr node, Arena& arena, Precedence p = Precedence::None) noexcept;
    static void CollectVariables(NodePtr node, SymbolTable& var_map) noexcept;
    static NodePtr Derivative(NodePtr node, Arena& arena, std::string_view var) noexcept;
    static NodePtr Simplify(NodePtr node, Arena& arena) noexcept;

#if defined(ASMJIT_BUILD_X86)
    static bool CompileX86(NodePtr node, asmjit::x86::Compiler& cc, asmjit::x86::Gp vars_ptr, const SymbolTable& var_map, asmjit::x86::Vec& out) noexcept;
#endif
};

} // namespace AXIOM
