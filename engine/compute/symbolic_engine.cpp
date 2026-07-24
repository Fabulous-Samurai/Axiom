// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "symbolic_engine.h"
#include "string_helpers.h"

#include <algorithm>
#include <ranges>
#include <cctype>
#include <cmath>
#include <charconv>
#include <utility>
#include <system_error>
#include "fixed_vector.h"

namespace AXIOM {

namespace {

bool IsBlank(std::string_view value) noexcept {
    return value.find_first_not_of(" \t\n\r") == std::string_view::npos;
}

std::string_view TrimSV(std::string_view value) noexcept {
    auto start = value.find_first_not_of(" \t\n\r");
    if (start == std::string_view::npos) return {};
    auto end = value.find_last_not_of(" \t\n\r");
    return value.substr(start, end - start + 1);
}

bool IsIdentifierChar(char c) noexcept {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

template<size_t N>
void AppendToBuffer(AXIOM::FixedVector<char, N>& buffer, std::string_view sv) noexcept {
    for (char c : sv) {
        buffer.push_back(c);
    }
}

template<size_t N>
void NumberToBuffer(double value, AXIOM::FixedVector<char, N>& buffer) noexcept {
    char temp[64];
    auto [ptr, ec] = std::to_chars(temp, temp + sizeof(temp), value, std::chars_format::fixed, 12);
    if (ec == std::errc{}) {
        std::string_view s(temp, ptr - temp);
        while (s.size() > 1 && s.back() == '0') {
            s.remove_suffix(1);
        }
        if (!s.empty() && s.back() == '.') {
            s.remove_suffix(1);
        }
        if (s.empty()) {
            buffer.push_back('0');
        } else {
            AppendToBuffer(buffer, s);
        }
    } else {
        buffer.push_back('0');
    }
}

std::string_view NumberToString(Arena& arena, double value) noexcept {
    AXIOM::FixedVector<char, 64> buf;
    NumberToBuffer(value, buf);
    return arena.allocString(std::string_view(buf.data(), buf.size()));
}

bool IsIntegerValue(double x) noexcept {
    return std::abs(x - std::round(x)) < 1e-9;
}

double BinomialCoeff(int n, int k) noexcept {
    if (k < 0 || k > n) {
        return 0.0;
    }
    if (k == 0 || k == n) {
        return 1.0;
    }
    k = std::min(k, n - k);
    double coeff = 1.0;
    for (int i = 1; i <= k; ++i) {
        coeff = coeff * static_cast<double>(n - (k - i)) / static_cast<double>(i);
    }
    return coeff;
}

bool ParseDoubleStrict(std::string_view token, double& out) noexcept {
    auto [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), out);
    return ec == std::errc{} && ptr == token.data() + token.size();
}

std::string_view ReplaceVariableTokens(Arena& arena, std::string_view expr, std::string_view var, std::string_view replacement) noexcept {
    AXIOM::FixedVector<char, 4096> out;
    for (size_t i = 0; i < expr.size();) {
        bool starts_token = (i == 0 || !IsIdentifierChar(expr[i - 1]));
        if (starts_token && i + var.size() <= expr.size() && expr.substr(i, var.size()) == var) {
            const size_t right = i + var.size();
            const bool ends_token = (right == expr.size() || !IsIdentifierChar(expr[right]));
            if (ends_token) {
                AppendToBuffer(out, replacement);
                i += var.size();
                continue;
            }
        }
        if (out.size() < out.capacity()) {
            out.push_back(expr[i]);
        }
        ++i;
    }
    return arena.allocString(std::string_view(out.data(), out.size()));
}

EngineResult EvalScalar(std::string_view expr, const AXIOM::SymbolTable& context = {}) noexcept {
    AXIOM::AlgebraicParser parser;
    if (context.empty()) {
        return parser.ParseAndExecute(expr);
    }
    return parser.ParseAndExecuteWithContext(expr, context);
}

bool EvalDouble(std::string_view expr, double& out, const AXIOM::SymbolTable& context = {}) noexcept {
    EngineResult res = EvalScalar(expr, context);
    auto value = res.GetDouble();
    if (!value.has_value() || !std::isfinite(*value)) {
        return false;
    }
    out = *value;
    return true;
}

bool BisectionRoot(std::string_view expr,
                  std::string_view var,
                  double left,
                  double right,
                  double& root) noexcept {
    AXIOM::SymbolTable ctx;
    auto eval = [&](double x, double& fx) -> bool {
        ctx.clear();
        ctx.push_back({var, AXIOM::Number(x)});
        return EvalDouble(expr, fx, ctx);
    };

    double f_left = 0.0;
    double f_right = 0.0;
    if (!eval(left, f_left) || !eval(right, f_right)) {
        return false;
    }
    if (std::abs(f_left) < 1e-10) {
        root = left;
        return true;
    }
    if (std::abs(f_right) < 1e-10) {
        root = right;
        return true;
    }
    if (f_left * f_right > 0.0) {
        return false;
    }

    for (int i = 0; i < 80; ++i) {
        const double mid = 0.5 * (left + right);
        double f_mid = 0.0;
        if (!eval(mid, f_mid)) {
            return false;
        }

        if (std::abs(f_mid) < 1e-10 || std::abs(right - left) < 1e-10) {
            root = mid;
            return true;
        }

        if (f_left * f_mid <= 0.0) {
            right = mid;
            f_right = f_mid;
        } else {
            left = mid;
            f_left = f_mid;
        }
    }

    root = 0.5 * (left + right);
    return true;
}

} // namespace

EngineResult SymbolicEngine::Expand(std::string_view expression) noexcept {
    arena_.reset();
    if (IsBlank(expression)) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    const std::string_view expr = TrimSV(expression);
    if (expr.size() >= 6 && expr.front() == '(' && expr.find(')') != std::string_view::npos && expr.back() >= '0' && expr.back() <= '9') {
        if (const size_t close = expr.find(')'); close + 2 < expr.size() && expr[close + 1] == '^') {
            const std::string_view inside = expr.substr(1, close - 1);
            const std::string_view power_s = expr.substr(close + 2);
            double power_d = 0.0;
            if (ParseDoubleStrict(power_s, power_d) && IsIntegerValue(power_d)) {
                if (const int n = static_cast<int>(std::round(power_d)); n >= 0 && n <= 16) {
                    size_t split = inside.find('+');
                    char op = '+';
                    if (split == std::string_view::npos) {
                        split = inside.find('-', 1);
                        op = '-';
                    }
                    if (split != std::string_view::npos) {
                        const std::string_view a = TrimSV(inside.substr(0, split));
                        const std::string_view b_raw = TrimSV(inside.substr(split + 1));
                        
                        double b_num = 0.0;
                        if (ParseDoubleStrict(b_raw, b_num)) {
                            if (op == '-') b_num = -b_num;
                            if (!a.empty() && std::isalpha(static_cast<unsigned char>(a[0]))) {
                                return CreateSuccessResult(ExpandBinomial(a, b_num, n));
                            }
                        }
                    }
                }
            }
        }
    }

    return CreateSuccessResult(arena_.allocString(expr));
}

EngineResult SymbolicEngine::Factor(std::string_view expression) noexcept {
    arena_.reset();
    if (IsBlank(expression)) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    const std::string_view trimmed = TrimSV(expression);
    AXIOM::FixedVector<char, 1024> candidate_buf;
    for (char ch : trimmed) {
        if (std::isspace(static_cast<unsigned char>(ch)) == 0) {
            candidate_buf.push_back(ch);
        }
    }
    std::string_view candidate(candidate_buf.data(), candidate_buf.size());

    // Minimal integer-root factorization for x^2+bx+c.
    if (size_t x2 = candidate.find("x^2"); x2 != std::string_view::npos) {
        std::string_view tail = candidate.substr(x2 + 3);
        if (size_t x_pos = tail.find('x'); x_pos != std::string_view::npos) {
            std::string_view b_str = tail.substr(0, x_pos);
            std::string_view c_str = tail.substr(x_pos + 1);
            if (b_str.empty() || b_str == "+") {
                b_str = "1";
            } else if (b_str == "-") {
                b_str = "-1";
            }

            if (std::string_view factored = FactorQuadratic(b_str, c_str); !factored.empty()) {
                return CreateSuccessResult(factored);
            }
        }
    }

    return CreateSuccessResult(arena_.allocString(trimmed));
}

EngineResult SymbolicEngine::Simplify(std::string_view expression) noexcept {
    arena_.reset();
    if (IsBlank(expression)) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    double value = 0.0;
    if (EvalDouble(expression, value)) {
        return CreateSuccessResult(NumberToString(arena_, value));
    }
    return CreateSuccessResult(arena_.allocString(TrimSV(expression)));
}

EngineResult SymbolicEngine::Substitute(std::string_view expr, std::string_view var, std::string_view value) noexcept {
    arena_.reset();
    if (IsBlank(expr) || IsBlank(var) || IsBlank(value)) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    const std::string_view trimmed_var = TrimSV(var);
    const std::string_view trimmed_val = TrimSV(value);
    
    AXIOM::FixedVector<char, 256> replacement;
    replacement.push_back('(');
    AppendToBuffer(replacement, trimmed_val);
    replacement.push_back(')');
    
    const std::string_view replaced = ReplaceVariableTokens(arena_, expr, trimmed_var, std::string_view(replacement.data(), replacement.size()));
    return CreateSuccessResult(replaced);
}

EngineResult SymbolicEngine::Integrate(std::string_view expression, std::string_view variable) noexcept {
    arena_.reset();
    if (IsBlank(expression) || IsBlank(variable)) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    const std::string_view expr = TrimSV(expression);
    const std::string_view var = TrimSV(variable);

    double c = 0.0;
    if (ParseDoubleStrict(expr, c)) {
        AXIOM::FixedVector<char, 128> res_buf;
        NumberToBuffer(c, res_buf);
        res_buf.push_back('*');
        AppendToBuffer(res_buf, var);
        return CreateSuccessResult(arena_.allocString(std::string_view(res_buf.data(), res_buf.size())));
    }

    if (expr == var) {
        AXIOM::FixedVector<char, 128> res_buf;
        AppendToBuffer(res_buf, "0.5*");
        AppendToBuffer(res_buf, var);
        AppendToBuffer(res_buf, "^2");
        return CreateSuccessResult(arena_.allocString(std::string_view(res_buf.data(), res_buf.size())));
    }

    if (expr.size() == var.size() + 5 && expr.starts_with("sin(") && expr.ends_with(")") && expr.substr(4, var.size()) == var) {
        AXIOM::FixedVector<char, 128> res_buf;
        AppendToBuffer(res_buf, "-cos(");
        AppendToBuffer(res_buf, var);
        AppendToBuffer(res_buf, ")");
        return CreateSuccessResult(arena_.allocString(std::string_view(res_buf.data(), res_buf.size())));
    }
    if (expr.size() == var.size() + 5 && expr.starts_with("cos(") && expr.ends_with(")") && expr.substr(4, var.size()) == var) {
        AXIOM::FixedVector<char, 128> res_buf;
        AppendToBuffer(res_buf, "sin(");
        AppendToBuffer(res_buf, var);
        AppendToBuffer(res_buf, ")");
        return CreateSuccessResult(arena_.allocString(std::string_view(res_buf.data(), res_buf.size())));
    }

    if (expr.starts_with(var) && expr.size() > var.size() && expr[var.size()] == '^') {
        double n = 0.0;
        if (ParseDoubleStrict(expr.substr(var.size() + 1), n) && std::abs(n + 1.0) > 1e-12) {
            const double p = n + 1.0;
            AXIOM::FixedVector<char, 256> res_buf;
            res_buf.push_back('(');
            NumberToBuffer(1.0 / p, res_buf);
            AppendToBuffer(res_buf, ")*");
            AppendToBuffer(res_buf, var);
            AppendToBuffer(res_buf, "^");
            NumberToBuffer(p, res_buf);
            return CreateSuccessResult(arena_.allocString(std::string_view(res_buf.data(), res_buf.size())));
        }
    }

    return CreateErrorResult(CalcErr::OperationNotFound);
}

EngineResult SymbolicEngine::DefiniteIntegral(std::string_view expr, std::string_view var, double a, double b) noexcept {
    arena_.reset();
    if (IsBlank(expr) || IsBlank(var)) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    AXIOM::FixedVector<char, 512> command;
    AppendToBuffer(command, "integrate(");
    AppendToBuffer(command, TrimSV(expr));
    AppendToBuffer(command, ", ");
    AppendToBuffer(command, TrimSV(var));
    AppendToBuffer(command, ", ");
    NumberToBuffer(a, command);
    AppendToBuffer(command, ", ");
    NumberToBuffer(b, command);
    AppendToBuffer(command, ")");
    
    EngineResult res = EvalScalar(std::string_view(command.data(), command.size()));
    if (res.HasResult()) {
        return res;
    }
    return CreateErrorResult(CalcErr::DomainError);
}

EngineResult SymbolicEngine::PartialDerivative(std::string_view expr, std::string_view var) noexcept {
    arena_.reset();
    if (IsBlank(expr) || IsBlank(var)) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    const std::string_view variable = TrimSV(var);
    const std::string_view source = TrimSV(expr);
    
    std::string_view derivative_target;
    if (variable != "x") {
        derivative_target = ReplaceVariableTokens(arena_, source, variable, "x");
    } else {
        derivative_target = source;
    }

    AXIOM::AlgebraicParser parser;
    AXIOM::FixedVector<char, 2048> cmd_buf;
    AppendToBuffer(cmd_buf, "derive ");
    AppendToBuffer(cmd_buf, derivative_target);
    
    EngineResult res = parser.ParseAndExecute(std::string_view(cmd_buf.data(), cmd_buf.size()));
    if (!res.result.has_value()) {
        return CreateErrorResult(CalcErr::OperationNotFound);
    }

    const std::string_view* deriv_sv = std::get_if<std::string_view>(&*res.result);
    if (deriv_sv == nullptr) {
        return CreateErrorResult(CalcErr::OperationNotFound);
    }

    if (variable != "x") {
        return CreateSuccessResult(ReplaceVariableTokens(arena_, *deriv_sv, "x", variable));
    }
    return CreateSuccessResult(*deriv_sv);
}

EngineResult SymbolicEngine::TaylorSeries(std::string_view expr, std::string_view var, double point, int order) noexcept {
    arena_.reset();
    if (IsBlank(expr) || IsBlank(var) || order < 0) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    const std::string_view variable = TrimSV(var);
    AXIOM::AlgebraicParser parser;
    auto current_ast = parser.ParseExpression(expr);
    if (!current_ast) return CreateErrorResult(CalcErr::ParseError);

    double fact = 1.0;
    AXIOM::FixedVector<char, 2048> series;
    bool any_term = false;

    for (int k = 0; k <= order; ++k) {
        CalculateTaylorTerm(series, variable, point, k, fact, current_ast, any_term);

        if (k < order) {
            current_ast = NodeDispatcher::Derivative(current_ast, parser.GetArena(), variable);
            if (!current_ast) break;
            current_ast = NodeDispatcher::Simplify(current_ast, parser.GetArena());
            fact *= static_cast<double>(k + 1);
        }
    }

    if (!any_term) {
        return CreateErrorResult(CalcErr::OperationNotFound);
    }
    return CreateSuccessResult(arena_.allocString(std::string_view(series.data(), series.size())));
}

EngineResult SymbolicEngine::SolveEquation(std::string_view equation, std::string_view variable) noexcept {
    arena_.reset();
    if (IsBlank(equation) || IsBlank(variable)) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    const std::string_view var = TrimSV(variable);
    AXIOM::FixedVector<char, 2048> expr_buf;
    BuildEquationExpression(expr_buf, TrimSV(equation));
    std::string_view expr(expr_buf.data(), expr_buf.size());

    AXIOM::FixedVector<double, 256> roots;
    const double left = -2000.0;
    const double right = 2000.0;
    const int steps = 1000;
    const double step = (right - left) / static_cast<double>(steps);

    AXIOM::SymbolTable ctx;
    auto eval = [&](double x, double& fx) -> bool {
        ctx.clear();
        ctx.push_back({var, AXIOM::Number(x)});
        return EvalDouble(expr, fx, ctx);
    };

    double prev_x = left;
    double prev_f = 0.0;
    bool prev_ok = eval(prev_x, prev_f);
    for (int i = 1; i <= steps; ++i) {
        const double x = left + i * step;
        double fx = 0.0;
        bool ok = eval(x, fx);
        if (ok && prev_ok) {
            if (prev_f * fx <= 0.0) {
                double root = 0.0;
                if (BisectionRoot(expr, var, prev_x, x, root)) {
                    roots.push_back(root);
                }
            }
        }
        prev_x = x;
        prev_f = fx;
        prev_ok = ok;
    }

    if (roots.empty()) {
        return CreateErrorResult(CalcErr::OperationNotFound);
    }

    std::ranges::sort(roots);
    AXIOM::FixedVector<double, 256> unique_roots;
    for (double r : roots) {
        if (unique_roots.empty() || std::abs(unique_roots.back() - r) > 1e-5) {
            unique_roots.push_back(r);
        }
    }
    return CreateSuccessResult(std::move(unique_roots));
}

EngineResult SymbolicEngine::SolveSystem(const AXIOM::FixedVector<std::string_view, 256>& equations, const AXIOM::FixedVector<std::string_view, 256>& variables) noexcept {
    arena_.reset();
    if (equations.empty() || variables.empty()) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }
    for (const auto& eq : equations) {
        if (IsBlank(eq)) {
            return CreateErrorResult(CalcErr::ArgumentMismatch);
        }
    }
    for (const auto& variable : variables) {
        if (IsBlank(variable)) {
            return CreateErrorResult(CalcErr::ArgumentMismatch);
        }
    }

    AXIOM::FixedVector<char, 4096> command;
    AppendToBuffer(command, "solve_nl {");
    for (size_t i = 0; i < equations.size(); ++i) {
        if (i > 0) {
            command.push_back(';');
        }
        AppendToBuffer(command, equations[i]);
    }
    AppendToBuffer(command, "} [");
    for (size_t i = 0; i < variables.size(); ++i) {
        if (i > 0) {
            command.push_back(',');
        }
        command.push_back('1');
    }
    command.push_back(']');

    AXIOM::AlgebraicParser parser;
    EngineResult res = parser.ParseAndExecute(std::string_view(command.data(), command.size()));
    if (res.HasResult()) {
        return res;
    }
    return CreateErrorResult(CalcErr::OperationNotFound);
}

EngineResult SymbolicEngine::FindLimits(std::string_view expr, std::string_view var, double approach_point) noexcept {
    arena_.reset();
    if (IsBlank(expr) || IsBlank(var)) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    AXIOM::FixedVector<char, 2048> cmd;
    AppendToBuffer(cmd, "limit(");
    AppendToBuffer(cmd, TrimSV(expr));
    AppendToBuffer(cmd, ", ");
    AppendToBuffer(cmd, TrimSV(var));
    AppendToBuffer(cmd, ", ");
    NumberToBuffer(approach_point, cmd);
    AppendToBuffer(cmd, ")");

    EngineResult res = EvalScalar(std::string_view(cmd.data(), cmd.size()));
    if (res.HasResult()) {
        return res;
    }
    return CreateErrorResult(CalcErr::DomainError);
}

EngineResult SymbolicEngine::FindRoots(std::string_view expr, std::string_view var, double range_min, double range_max) noexcept {
    arena_.reset();
    if (IsBlank(expr) || IsBlank(var) || range_min > range_max) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    AXIOM::FixedVector<double, 256> roots;
    const int samples = 256;
    const double step = (range_max - range_min) / static_cast<double>(samples);
    const std::string_view variable = TrimSV(var);

    AXIOM::SymbolTable ctx;
    auto eval = [&](double x, double& fx) -> bool {
        ctx.clear();
        ctx.push_back({variable, AXIOM::Number(x)});
        return EvalDouble(expr, fx, ctx);
    };

    double prev_x = range_min;
    double prev_f = 0.0;
    bool prev_ok = eval(prev_x, prev_f);
    for (int i = 1; i <= samples; ++i) {
        const double x = range_min + step * static_cast<double>(i);
        double fx = 0.0;
        const bool ok = eval(x, fx);
        if (ok && prev_ok) {
            if (std::abs(fx) < 1e-8) {
                roots.push_back(x);
            } else if (prev_f * fx < 0.0) {
                double root = 0.0;
                if (BisectionRoot(expr, variable, prev_x, x, root)) {
                    roots.push_back(root);
                }
            }
        }
        prev_x = x;
        prev_f = fx;
        prev_ok = ok;
    }

    if (roots.empty()) {
        return CreateErrorResult(CalcErr::OperationNotFound);
    }
    std::ranges::sort(roots);
    AXIOM::FixedVector<double, 256> unique_roots;
    for (double r : roots) {
        if (unique_roots.empty() || std::abs(unique_roots.back() - r) > 1e-5) {
            unique_roots.push_back(r);
        }
    }
    return CreateSuccessResult(std::move(unique_roots));
}

std::string_view SymbolicEngine::ExpandBinomial(std::string_view var_name, double b_num, int n) noexcept {
    AXIOM::FixedVector<char, 2048> out;
    bool first = true;
    for (int k = 0; k <= n; ++k) {
        const double coeff = BinomialCoeff(n, k) * std::pow(b_num, k);
        const int var_pow = n - k;
        if (std::abs(coeff) < 1e-12) {
            continue;
        }

        if (!first) {
            AppendToBuffer(out, coeff >= 0.0 ? " + " : " - ");
        } else if (coeff < 0.0) {
            out.push_back('-');
        }

        const double abs_coeff = std::abs(coeff);
        const bool emit_coeff = (var_pow == 0) || std::abs(abs_coeff - 1.0) > 1e-12;
        if (emit_coeff) {
            NumberToBuffer(abs_coeff, out);
            if (var_pow > 0) {
                out.push_back('*');
            }
        }
        if (var_pow > 0) {
            AppendToBuffer(out, var_name);
            if (var_pow > 1) {
                AppendToBuffer(out, "^");
                NumberToBuffer(static_cast<double>(var_pow), out);
            }
        }

        first = false;
    }
    return arena_.allocString(std::string_view(out.data(), out.size()));
}

std::string_view SymbolicEngine::FactorQuadratic(std::string_view b_str, std::string_view c_str) noexcept {
    double b = 0.0, c = 0.0;
    if (ParseDoubleStrict(b_str, b) && ParseDoubleStrict(c_str, c) && IsIntegerValue(b) && IsIntegerValue(c)) {
        const int bi = static_cast<int>(std::round(b));
        const int ci = static_cast<int>(std::round(c));
        for (int p = -64; p <= 64; ++p) {
            if (p == 0 || (ci != 0 && ci % p != 0)) continue;
            const int q = (ci == 0) ? (bi - p) : (ci / p);
            if (p + q == bi && p * q == ci) {
                AXIOM::FixedVector<char, 256> out;
                AppendToBuffer(out, "(x");
                if (p >= 0) AppendToBuffer(out, " + "); else AppendToBuffer(out, " - ");
                NumberToBuffer(std::abs(static_cast<double>(p)), out);
                AppendToBuffer(out, ")*(x");
                if (q >= 0) AppendToBuffer(out, " + "); else AppendToBuffer(out, " - ");
                NumberToBuffer(std::abs(static_cast<double>(q)), out);
                AppendToBuffer(out, ")");
                return arena_.allocString(std::string_view(out.data(), out.size()));
            }
        }
    }
    return {};
}

bool SymbolicEngine::CalculateTaylorTerm(FixedVector<char, 2048>& series,
                                        std::string_view variable,
                                        double point,
                                        int k,
                                        double fact,
                                        NodePtr ast,
                                        bool& any_term) noexcept {
    SymbolTable context;
    context.push_back({std::string(variable), AXIOM::Number(point)});

    auto res = NodeDispatcher::Evaluate(ast, context);
    if (res.HasValue()) {
        const double coeff = GetReal(*res.value) / fact;
        if (std::abs(coeff) > 1e-12) {
            if (any_term) {
                AppendToBuffer(series, coeff >= 0.0 ? " + " : " - ");
            } else if (coeff < 0.0) {
                series.push_back('-');
            }
            const double abs_coeff = std::abs(coeff);
            if (k == 0) {
                NumberToBuffer(abs_coeff, series);
            } else {
                if (std::abs(abs_coeff - 1.0) > 1e-12) {
                    NumberToBuffer(abs_coeff, series);
                    series.push_back('*');
                }
                series.push_back('(');
                AppendToBuffer(series, variable);
                AppendToBuffer(series, " - ");
                NumberToBuffer(point, series);
                series.push_back(')');
                if (k > 1) {
                    series.push_back('^');
                    NumberToBuffer(static_cast<double>(k), series);
                }
            }
            any_term = true;
            return true;
        }
    }
    return false;
}

void SymbolicEngine::BuildEquationExpression(AXIOM::FixedVector<char, 2048>& buffer, std::string_view eq) noexcept {
    if (const size_t equal_pos = eq.find('='); equal_pos != std::string_view::npos) {
        buffer.push_back('(');
        AppendToBuffer(buffer, eq.substr(0, equal_pos));
        AppendToBuffer(buffer, ")-(");
        AppendToBuffer(buffer, eq.substr(equal_pos + 1));
        buffer.push_back(')');
    } else {
        AppendToBuffer(buffer, eq);
    }
}

} // namespace AXIOM
