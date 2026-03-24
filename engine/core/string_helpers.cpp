// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "string_helpers.h"
#include <sstream>

namespace Utils {

std::string Trim(std::string_view str) {
    const auto first = str.find_first_not_of(" \t\n\r");
    if (first == std::string_view::npos) return {};
    const auto last = str.find_last_not_of(" \t\n\r");
    return std::string(str.substr(first, last - first + 1));
}

std::vector<std::string> Split(std::string_view s, char delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    while (start <= s.size()) {
        const auto pos = s.find(delimiter, start);
        const auto len = (pos == std::string_view::npos) ? s.size() - start : pos - start;
        const auto part = Trim(s.substr(start, len));
        if (!part.empty()) {
            tokens.push_back(part);
        }
        if (pos == std::string_view::npos) break;
        start = pos + 1;
    }
    return tokens;
}

std::string ReplaceAll(std::string_view str, std::string_view from, std::string_view to) {
    if (from.empty()) return std::string(str);
    std::string result;
    result.reserve(str.size());
    size_t pos = 0;
    while (true) {
        const auto found = str.find(from, pos);
        result.append(str.substr(pos, (found == std::string_view::npos) ? str.size() - pos : found - pos));
        if (found == std::string_view::npos) break;
        result.append(to);
        pos = found + from.size();
    }
    return result;
}

}
