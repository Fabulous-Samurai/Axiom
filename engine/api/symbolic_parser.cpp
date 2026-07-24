// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "symbolic_parser.h"
#include "symbolic_engine.h"
#include "string_helpers.h"
#include <algorithm>
#include <cctype>

namespace AXIOM {

static bool try_parse_double(std::string_view_view s, double& out) noexcept {
    auto val = Utils::FastParseDouble(s);
    if (val) {
        out = *val;
        return true;
    }
    return false;
}

EngineResult SymbolicParser::ParseAndExecute(std::string_view_view input) noexcept {
    std::string_view s = std::string_view(input);
    std::string_view lower;
    lower.reserve(s.size());
    for (char c : s) lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));

    // expand(expr)
    if (lower.find("expand(") == 0) {
        size_t lp = s.find('(');
        size_t rp = s.rfind(')');
        if (lp != std::string_view::npos && rp != std::string_view::npos && rp > lp) {
            return engine_->Expand(s.substr(lp + 1, rp - lp - 1));
        }
    }

    // simplify(expr)
    if (lower.find("simplify(") == 0) {
        size_t lp = s.find('(');
        size_t rp = s.rfind(')');
        if (lp != std::string_view::npos && rp != std::string_view::npos && rp > lp) {
            return engine_->Simplify(s.substr(lp + 1, rp - lp - 1));
        }
    }

    // solve([eq1, eq2], [x, y])
    if (lower.find("solve(") == 0) {
        size_t lp = s.find('(');
        size_t rp = s.rfind(')');
        if (lp != std::string_view::npos && rp != std::string_view::npos && rp > lp) {
            std::string_view args = s.substr(lp + 1, rp - lp - 1);
            size_t mid = args.find("],[");
            if (mid != std::string_view::npos) {
                std::string_view eqs_part = args.substr(0, mid + 1);
                std::string_view vars_part = args.substr(mid + 1);

                FixedVector<std::string_view_view, 256> equations;
                FixedVector<std::string_view_view, 256> variables;

                // Simple split logic for [a,b]
                auto parse_list = [](std::string_view_view part, FixedVector<std::string_view_view, 256>& out) {
                    size_t lb = part.find('[');
                    size_t rb = part.rfind(']');
                    if (lb == std::string_view::npos || rb == std::string_view::npos) return;
                    std::string_view_view body = part.substr(lb + 1, rb - lb - 1);
                    size_t pos = 0;
                    while (pos < body.size()) {
                        size_t next = body.find(',', pos);
                        out.push_back(Utils::Trim(body.substr(pos, next == std::string_view_view::npos ? std::string_view_view::npos : next - pos)));
                        if (next == std::string_view_view::npos) break;
                        pos = next + 1;
                    }
                };

                parse_list(eqs_part, equations);
                parse_list(vars_part, variables);

                return engine_->SolveSystem(equations, variables);
            }
        }
    }

    return CreateErrorResult(CalcErr::OperationNotFound);
}

} // namespace AXIOM
