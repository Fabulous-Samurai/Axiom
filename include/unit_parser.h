#pragma once
#include "IParser.h"
#include "unit_manager.h"
#include <string>
#include <regex>

namespace AXIOM {

class UnitParser : public IParser {
private:
    UnitManager* unit_manager_;

public:
    explicit UnitParser(UnitManager* manager) noexcept : unit_manager_(manager) {}

    // Backwards-compatible virtual override (existing callers)
    EngineResult ParseAndExecute(const std::string& input) override;

    // New, non-virtual view-based APIs for zero-copy call sites
    EngineResult ParseAndExecuteView(std::string_view input);

private:
    bool IsUnitConversion(std::string_view input);
    EngineResult ParseConversion(std::string_view input);
};

} // namespace AXIOM
