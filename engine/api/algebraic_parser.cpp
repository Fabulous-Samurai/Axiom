// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "ast_nodes.h"
#include "algebraic_parser.h"
#include "string_helpers.h"
#include "telemetry.h"
#include <cmath>
#include "arena_allocator.h"
#include "fixed_vector.h"
#include <cctype>

namespace AXIOM {

namespace {
    struct ParserState {
        std::string_view input;
        size_t pos{0};
        Arena& arena;
        int depth{0};
        static constexpr int MAX_DEPTH = 512;

        void skip_ws() noexcept {
            while (pos < input.length() && std::isspace(static_cast<unsigned char>(input[pos]))) pos++;
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
            explicit DepthGuard(ParserState& state) noexcept : s(state) {
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
        while (state.has_more() && (std::isdigit(static_cast<unsigned char>(state.peek())) || state.peek() == '.')) state.get();
        auto val_opt = Utils::FastParseDouble(state.input.substr(start, state.pos - start));
        double val = val_opt.value_or(0.0);
        return state.arena.template alloc<AnyNode>(NumberNode(val));
    }

    NodePtr parse_identifier(ParserState& state) noexcept {
        size_t start = state.pos;
        while (state.has_more() && (std::isalnum(static_cast<unsigned char>(state.peek())) || state.peek() == '_')) state.get();
        std::string_view name = state.input.substr(start, state.pos - start);
        
        state.skip_ws();
        if (state.peek() == '(') {
            state.get(); // '('
            auto arg = parse_expression(state);
            if (!arg) return nullptr;
            state.match(')');
            return state.arena.template alloc<AnyNode>(UnaryOpNode(state.arena.allocString(name), arg));
        }
        
        return state.arena.template alloc<AnyNode>(VariableNode(state.arena.allocString(name)));
    }

    void parse_matrix_row(ParserState& state, Matrix& m) noexcept {
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
    }

    NodePtr parse_matrix(ParserState& state) noexcept {
        state.get(); // '['
        Matrix m;
        state.skip_ws();
        while (state.has_more() && state.peek() != ']') {
            parse_matrix_row(state, m);
            state.match(',');
            state.skip_ws();
        }
        state.match(']');
        return state.arena.template alloc<AnyNode>(MatrixNode(std::move(m)));
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

        if (std::isdigit(static_cast<unsigned char>(p)) || p == '.') {
            return parse_number(state);
        }

        if (std::isalpha(static_cast<unsigned char>(p))) {
            return parse_identifier(state);
        }

        if (p == '-') {
            state.pos++;
            auto arg = parse_primary(state);
            if (!arg) return nullptr;
            return state.arena.template alloc<AnyNode>(UnaryOpNode("u-", arg));
        }

        return state.arena.template alloc<AnyNode>(NumberNode(0.0));
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
            left = state.arena.template alloc<AnyNode>(BinaryOpNode('^', left, right));
            state.skip_ws();
        }
        return left;
    }

    bool is_matrix_node(NodePtr node) noexcept {
        if (!node) return false;
        return std::holds_alternative<MatrixNode>(*node) || 
               std::holds_alternative<MatrixBinaryOpNode>(*node);
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
            if (is_matrix_node(left) || is_matrix_node(right)) {
                left = state.arena.template alloc<AnyNode>(MatrixBinaryOpNode(op, left, right));
            } else {
                left = state.arena.template alloc<AnyNode>(BinaryOpNode(op, left, right));
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
            if (is_matrix_node(left) || is_matrix_node(right)) {
                left = state.arena.template alloc<AnyNode>(MatrixBinaryOpNode(op, left, right));
            } else {
                left = state.arena.template alloc<AnyNode>(BinaryOpNode(op, left, right));
            }
            state.skip_ws();
        }
        return left;
    }

    bool ParseLimitArguments(std::string_view input, std::string_view& expr, std::string_view& var, double& point) noexcept {
        auto start = input.find('(');
        auto end = input.rfind(')');
        if (start == std::string_view::npos || end == std::string_view::npos || end <= start) return false;
        
        std::string_view content = input.substr(start + 1, end - start - 1);
        size_t c1 = content.find(',');
        if (c1 == std::string_view::npos) return false;
        
        expr = Utils::Trim(content.substr(0, c1));
        std::string_view rest = content.substr(c1 + 1);
        size_t c2 = rest.find(',');
        if (c2 == std::string_view::npos) return false;
        
        var = Utils::Trim(rest.substr(0, c2));
        std::string_view point_str = Utils::Trim(rest.substr(c2 + 1));
        auto point_val = Utils::FastParseDouble(point_str);
        if (!point_val) return false;
        
        point = *point_val;
        return true;
    }

    bool ParseIntegrateArguments(std::string_view input, std::string_view& expr, std::string_view& var, double& a, double& b) noexcept {
        auto start = input.find('(');
        auto end = input.rfind(')');
        if (start == std::string_view::npos || end == std::string_view::npos || end <= start) return false;
        
        std::string_view content = input.substr(start + 1, end - start - 1);
        size_t c1 = content.find(',');
        if (c1 == std::string_view::npos) return false;
        
        expr = Utils::Trim(content.substr(0, c1));
        std::string_view rest1 = content.substr(c1 + 1);
        size_t c2 = rest1.find(',');
        if (c2 == std::string_view::npos) return false;
        
        var = Utils::Trim(rest1.substr(0, c2));
        std::string_view rest2 = rest1.substr(c2 + 1);
        size_t c3 = rest2.find(',');
        if (c3 == std::string_view::npos) return false;
        
        std::string_view a_str = Utils::Trim(rest2.substr(0, c3));
        std::string_view b_str = Utils::Trim(rest2.substr(c3 + 1));
        auto a_val = Utils::FastParseDouble(a_str);
        auto b_val = Utils::FastParseDouble(b_str);
        if (!a_val || !b_val) return false;
        
        a = *a_val;
        b = *b_val;
        return true;
    }
}

void AlgebraicParser::RegisterSpecialCommands() noexcept {}
NodePtr AlgebraicParser::ParseExpression(std::string_view input) noexcept { ParserState state{input, 0, arena_}; return parse_expression(state); }

EngineResult AlgebraicParser::ParseAndExecute(std::string_view input) noexcept { 
    std::string_view trimmed = Utils::Trim(input);
    if (trimmed.rfind("derive ", 0) == 0) return HandleDerivative(trimmed);
    if (trimmed.rfind("limit(", 0) == 0) return HandleLimit(trimmed);
    if (trimmed.rfind("integrate(", 0) == 0) return HandleIntegrate(trimmed);
    return ParseAndExecuteWithContext(trimmed, SymbolTable{}); 
}

EngineResult AlgebraicParser::HandleLimit(std::string_view input) noexcept {
    std::string_view expr, var;
    double point = 0;
    if (!ParseLimitArguments(input, expr, var, point)) return CreateErrorResult(CalcErr::ParseError);
    SymbolTable ctx; ctx.push_back({var, Number(point)});
    return ParseAndExecuteWithContext(expr, ctx);
}

EngineResult AlgebraicParser::HandleIntegrate(std::string_view input) noexcept {
    std::string_view expr, var;
    double a = 0, b = 0;
    if (!ParseIntegrateArguments(input, expr, var, a, b)) return CreateErrorResult(CalcErr::ParseError);
    
    constexpr int n = 1000;
    const double h = (b - a) / n;
    double sum = 0;
    SymbolTable ctx; ctx.push_back({var, 0.0});
    
    auto eval = [&](double x) { 
        if (!ctx.empty()) ctx[0].second = Number(x); 
        auto res = ParseAndExecuteWithContext(expr, ctx); 
        return res.HasResult() ? res.GetDouble().value_or(0.0) : 0.0; 
    };
    
    sum += 0.5 * (eval(a) + eval(b));
    for (int i = 1; i < n; i++) sum += eval(a + i * h);
    return CreateSuccessResult(sum * h);
}

EngineResult AlgebraicParser::ParseAndExecuteWithContext(std::string_view input, const SymbolTable& context) noexcept {
    auto root = ParseExpression(input); if (!root) return CreateErrorResult(CalcErr::StackOverflow);
    auto res = NodeDispatcher::Evaluate(root, context); if (res.HasValue()) {
        if (res.matrix.has_value()) return CreateSuccessResult(std::move(*res.matrix));
        if (res.vector.has_value()) return CreateSuccessResult(std::move(*res.vector));
        if (res.value.has_value()) return CreateSuccessResult(*res.value);
    }
    return CreateErrorResult(res.error);
}

EngineResult AlgebraicParser::HandleDerivative(std::string_view input) noexcept {
    // [FIX]: Use a transient (temporary) arena for this symbolic operation
    // to prevent dangling pointers from nodes created during simplification.
    Arena transient_arena(1 * 1024 * 1024); // 1MB arena for this operation only

    std::string_view target = input; if (target.rfind("derive ", 0) == 0) target = target.substr(7);
    
    // Parse expression using the transient arena
    ParserState state{target, 0, transient_arena};
    auto root = parse_expression(state); 
    if (!root) return CreateErrorResult(CalcErr::ParseError);

    // Perform symbolic operations on the transient arena
    auto deriv = NodeDispatcher::Derivative(root, transient_arena, "x"); 
    if (!deriv) return CreateErrorResult(CalcErr::OperationNotFound);
    
    auto simplified = NodeDispatcher::Simplify(deriv, transient_arena); 
    
    // The final result is a string, which is safe to return as it's copied.
    return CreateSuccessResult(NodeDispatcher::ToString(simplified, transient_arena));
}

EngineResult AlgebraicParser::HandleQuadratic(std::string_view input) noexcept { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult AlgebraicParser::HandleNonLinearSolve(std::string_view input) noexcept { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult AlgebraicParser::SolveQuadratic(double a, double b, double c) noexcept { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult AlgebraicParser::SolveNonLinearSystem(const FixedVector<std::string_view, 256>& equations, SymbolTable& guess) noexcept { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult AlgebraicParser::HandlePlotFunction(std::string_view input) noexcept { return CreateErrorResult(CalcErr::OperationNotFound); }

} // namespace AXIOM
