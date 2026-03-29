// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "unit_parser.h"
#include "string_helpers.h"
#include <regex>
#include <sstream>

namespace AXIOM {

// Lazy evaluate regex to prevent static initialization ordering crashes
static const std::regex& GetUnitConversionPattern() {
    static const std::regex kPattern(R"((convert\s+)?([0-9]+(?:\.[0-9]+)?)\s+([a-zA-Z]+)\s+to\s+([a-zA-Z]+))");
    return kPattern;
}

EngineResult UnitParser::ParseAndExecute(std::string_view input) noexcept {
    return ParseAndExecuteView(input);
}

EngineResult UnitParser::ParseAndExecuteView(std::string_view input) noexcept {
    if (IsUnitConversion(input)) {
        return ParseConversion(input);
    }
    return CreateErrorResult(CalcErr::ParseError);
}

bool UnitParser::IsUnitConversion(std::string_view input) noexcept {
    std::string s(input);
    return std::regex_search(s, GetUnitConversionPattern());
}

EngineResult UnitParser::ParseConversion(std::string_view input) noexcept {
    std::string s(input);
    std::smatch matches;

    if (std::regex_search(s, matches, GetUnitConversionPattern())) {
        auto value_opt = Utils::FastParseDouble(matches[2].str());
        if (!value_opt) return CreateErrorResult(CalcErr::ParseError);
        
        double value = *value_opt;
        std::string from_unit = matches[3].str();
        std::string to_unit = matches[4].str();

        return unit_manager_->ConvertUnit(value, from_unit, to_unit);
    }

    return CreateErrorResult(CalcErr::ParseError);
}

} // namespace AXIOM
