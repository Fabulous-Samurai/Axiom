// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "statistics_parser.h"
#include "string_helpers.h"
#include <algorithm>
#include <cctype>

namespace AXIOM {

AXIOM::FixedVector<double, 256> StatisticsParser::ParseVector(const std::string& s) {
    AXIOM::FixedVector<double, 256> out;
    size_t lb = s.find('[');
    size_t rb = s.rfind(']');
    if (lb == std::string::npos || rb == std::string::npos || rb <= lb) return out;

    std::string body = s.substr(lb + 1, rb - lb - 1);
    size_t pos = 0;
    while (pos < body.size()) {
        size_t next = body.find_first_of(",;", pos);
        std::string_view token = Utils::Trim(std::string_view(body).substr(pos, next == std::string::npos ? std::string_view::npos : next - pos));
        if (!token.empty()) {
            if (auto val = Utils::FastParseDouble(token)) {
                out.push_back(*val);
            }
        }
        if (next == std::string::npos) break;
        pos = next + 1;
    }
    return out;
}

EngineResult StatisticsParser::ParseAndExecute(std::string_view input) noexcept {
    std::string s = std::string(input);
    std::string lower;
    lower.reserve(s.size());
    for (char c : s) lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));

    if (lower.find("mean") == 0 || lower.find("statsmean") == 0 || lower.find("stats mean") == 0) {
        return engine_->Mean(ParseVector(s));
    }
    if (lower.find("variance") == 0 || lower.find("statsvariance") == 0 || lower.find("stats variance") == 0) {
        return engine_->Variance(ParseVector(s));
    }
    if (lower.find("std") == 0 || lower.find("standarddeviation") == 0 || lower.find("statsstd") == 0) {
        return engine_->StandardDeviation(ParseVector(s));
    }
    if (lower.find("median") == 0 || lower.find("statsmedian") == 0) {
        return engine_->Median(ParseVector(s));
    }
    if (lower.find("mode") == 0 || lower.find("statsmode") == 0) {
        return engine_->Mode(ParseVector(s));
    }

    if (lower.find("correlation(") == 0 || lower.find("statscorrelation(") == 0) {
        size_t lp = s.find('(');
        size_t rp = s.rfind(')');
        if (lp != std::string::npos && rp != std::string::npos && rp > lp) {
            std::string args = s.substr(lp + 1, rp - lp - 1);
            size_t mid = args.find("],[");
            if (mid != std::string::npos) {
                std::string xpart = args.substr(0, mid + 1);
                std::string ypart = args.substr(mid + 1);
                return engine_->Correlation(ParseVector(xpart), ParseVector(ypart));
            }
        }
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }

    return CreateErrorResult(CalcErr::OperationNotFound);
}

} // namespace AXIOM
