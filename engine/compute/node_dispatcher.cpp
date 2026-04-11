// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "ast_nodes.h"
#include <cmath>
#include <numbers>
#include <algorithm>
#include <cstdio>

namespace AXIOM {

namespace {
    constexpr double PI_CONST = std::numbers::pi;
    constexpr double D2R = std::numbers::pi / 180.0;
    constexpr double R2D = 180.0 / std::numbers::pi;

    int LookupSymbol(const SymbolTable& vars, std::string_view name) noexcept {
        for (size_t i = 0; i < vars.size(); ++i) {
            if (vars[i].first == name) return static_cast<int>(i);
        }
        return -1;
    }

    std::string_view FormatNumber(Arena& arena, double val) noexcept {
        char buf[64];
        int len = 0;
        if (std::isinf(val)) len = snprintf(buf, sizeof(buf), std::signbit(val) ? "-inf" : "inf");
        else if (std::isnan(val)) len = snprintf(buf, sizeof(buf), "nan");
        else if (val == std::floor(val) && std::abs(val) < 1e15) len = snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(val));
        else {
            const double abs_val = std::abs(val);
            if (abs_val >= 1e6 || (abs_val > 0 && abs_val < 1e-6)) len = snprintf(buf, sizeof(buf), "%.6e", val);
            else len = snprintf(buf, sizeof(buf), "%.15g", val);
        }
        return arena.allocString(std::string_view(buf, len > 0 ? static_cast<size_t>(len) : 0));
    }
}

EvalResult NodeDispatcher::Evaluate(NodePtr node, const SymbolTable& vars) noexcept {
    if (!node) return EvalResult::Failure(CalcErr::ArgumentMismatch);

    return std::visit([&](auto&& n) -> EvalResult {
        using T = std::decay_t<decltype(n)>;

        if constexpr (std::is_same_v<T, NumberNode>) {
            return EvalResult::Success(n.value);
        }
        else if constexpr (std::is_same_v<T, VariableNode>) {
            int idx = LookupSymbol(vars, n.name);
            if (idx != -1) return EvalResult::Success(vars[idx].second);
            if (n.name == "pi") return EvalResult::Success(PI_CONST);
            if (n.name == "e") return EvalResult::Success(std::numbers::e);
            return EvalResult::Failure(CalcErr::OperationNotFound);
        }
        else if constexpr (std::is_same_v<T, BinaryOpNode>) {
            auto l_res = Evaluate(n.left, vars); if (!l_res.HasValue()) return l_res;
            auto r_res = Evaluate(n.right, vars); if (!r_res.HasValue()) return r_res;
            double l = GetReal(*l_res.value), r = GetReal(*r_res.value);
            switch(n.op) {
                case '+': return EvalResult::Success(l + r);
                case '-': return EvalResult::Success(l - r);
                case '*': return EvalResult::Success(l * r);
                case '/': return (r == 0) ? EvalResult::Failure(CalcErr::DivideByZero) : EvalResult::Success(l / r);
                case '^': return EvalResult::Success(std::pow(l, r));
                default: return EvalResult::Failure(CalcErr::OperationNotFound);
            }
        }
        else if constexpr (std::is_same_v<T, UnaryOpNode>) {
            auto res = Evaluate(n.operand, vars); if (!res.HasValue()) return res;
            double v = GetReal(*res.value);
            if (n.func == "sin") return EvalResult::Success(std::sin(v * D2R));
            if (n.func == "cos") return EvalResult::Success(std::cos(v * D2R));
            if (n.func == "tan") return EvalResult::Success(std::tan(v * D2R));
            if (n.func == "sqrt") return (v < 0) ? EvalResult::Failure(CalcErr::NegativeRoot) : EvalResult::Success(std::sqrt(v));
            if (n.func == "abs") return EvalResult::Success(std::abs(v));
            if (n.func == "log") return (v <= 0) ? EvalResult::Failure(CalcErr::DomainError) : EvalResult::Success(std::log10(v));
            if (n.func == "exp") return EvalResult::Success(std::exp(v));
            if (n.func == "u-") return EvalResult::Success(-v);
            return EvalResult::Failure(CalcErr::OperationNotFound);
        }
        else if constexpr (std::is_same_v<T, MatrixNode>) {
            return EvalResult::Success(std::move(const_cast<Matrix&>(n.matrix)));
        }
        else if constexpr (std::is_same_v<T, MatrixBinaryOpNode>) {
            auto l_res = Evaluate(n.left, vars); if (!l_res.HasValue()) return l_res;
            auto r_res = Evaluate(n.right, vars); if (!r_res.HasValue()) return r_res;
            if (!l_res.matrix.has_value() || !r_res.matrix.has_value()) return EvalResult::Failure(CalcErr::ArgumentMismatch);
            const auto& A = *l_res.matrix; const auto& B = *r_res.matrix;
            if (n.op == '*') {
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
            return EvalResult::Failure(CalcErr::OperationNotFound);
        }
        return EvalResult::Failure(CalcErr::OperationNotFound);
    }, *node);
}

void NodeDispatcher::CollectVariables(NodePtr node, SymbolTable& var_map) noexcept {
    if (!node) return;
    std::visit([&](auto&& n) {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, VariableNode>) {
            if (n.name != "pi" && n.name != "e") {
                if (LookupSymbol(var_map, n.name) == -1) var_map.push_back({n.name, Number(0.0)});
            }
        } else if constexpr (std::is_same_v<T, BinaryOpNode>) {
            CollectVariables(n.left, var_map);
            CollectVariables(n.right, var_map);
        } else if constexpr (std::is_same_v<T, UnaryOpNode>) {
            CollectVariables(n.operand, var_map);
        } else if constexpr (std::is_same_v<T, MultiArgFunctionNode>) {
            for (auto arg : n.args) CollectVariables(arg, var_map);
        } else if constexpr (std::is_same_v<T, MatrixBinaryOpNode>) {
            CollectVariables(n.left, var_map);
            CollectVariables(n.right, var_map);
        }
    }, *node);
}

std::string_view NodeDispatcher::ToString(NodePtr node, Arena& arena, Precedence p) noexcept {
    if (!node) return "";
    return std::visit([&](auto&& n) -> std::string_view {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, NumberNode>) {
            return FormatNumber(arena, n.value);
        } else if constexpr (std::is_same_v<T, VariableNode>) {
            return n.name;
        } else if constexpr (std::is_same_v<T, BinaryOpNode>) {
            auto l = ToString(n.left, arena);
            auto r = ToString(n.right, arena);
            char buf[1024];
            int len = snprintf(buf, sizeof(buf), "%.*s %c %.*s", (int)l.length(), l.data(), n.op, (int)r.length(), r.data());
            return arena.allocString(std::string_view(buf, len > 0 ? static_cast<size_t>(len) : 0));
        } else if constexpr (std::is_same_v<T, UnaryOpNode>) {
            auto arg = ToString(n.operand, arena);
            char buf[1024];
            int len = snprintf(buf, sizeof(buf), "%.*s(%.*s)", (int)n.func.length(), n.func.data(), (int)arg.length(), arg.data());
            return arena.allocString(std::string_view(buf, len > 0 ? static_cast<size_t>(len) : 0));
        }
        return "unimplemented";
    }, *node);
}

NodePtr NodeDispatcher::Derivative(NodePtr node, Arena& arena, std::string_view var) noexcept {
    if (!node) return nullptr;
    return std::visit([&](auto&& n) -> NodePtr {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, NumberNode>) {
            return arena.template alloc<AnyNode>(NumberNode(0.0));
        } else if constexpr (std::is_same_v<T, VariableNode>) {
            return (n.name == var) ? arena.template alloc<AnyNode>(NumberNode(1.0)) : arena.template alloc<AnyNode>(NumberNode(0.0));
        } else if constexpr (std::is_same_v<T, BinaryOpNode>) {
            auto dl = Derivative(n.left, arena, var);
            auto dr = Derivative(n.right, arena, var);
            if (n.op == '+' || n.op == '-') return arena.template alloc<AnyNode>(BinaryOpNode(n.op, dl, dr));
            if (n.op == '*') {
                auto term1 = arena.template alloc<AnyNode>(BinaryOpNode('*', dl, n.right));
                auto term2 = arena.template alloc<AnyNode>(BinaryOpNode('*', n.left, dr));
                return arena.template alloc<AnyNode>(BinaryOpNode('+', term1, term2));
            }
            return arena.template alloc<AnyNode>(NumberNode(0.0));
        }
        return arena.template alloc<AnyNode>(NumberNode(0.0));
    }, *node);
}

NodePtr NodeDispatcher::Simplify(NodePtr node, Arena& arena) noexcept {
    if (!node) return nullptr;
    return std::visit([&](auto&& n) -> NodePtr {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, BinaryOpNode>) {
            auto sl = Simplify(n.left, arena);
            auto sr = Simplify(n.right, arena);
            return arena.template alloc<AnyNode>(BinaryOpNode(n.op, sl, sr));
        } else if constexpr (std::is_same_v<T, UnaryOpNode>) {
            return arena.template alloc<AnyNode>(UnaryOpNode(n.func, Simplify(n.operand, arena)));
        }
        return node;
    }, *node);
}

} // namespace AXIOM
