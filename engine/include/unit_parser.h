// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#pragma once
#include <regex>
#include <string>

#include "IParser.h"
#include "unit_manager.h"

namespace AXIOM {

class UnitParser : public IParser {
 private:
  UnitManager* unit_manager_;

 public:
  explicit UnitParser(UnitManager* manager) noexcept : unit_manager_(manager) {}

  // Backwards-compatible virtual override (existing callers)
  EngineResult ParseAndExecute(std::string_view input) noexcept override;

  // New, non-virtual view-based APIs for zero-copy call sites
  EngineResult ParseAndExecuteView(std::string_view input) noexcept;

 private:
  bool IsUnitConversion(std::string_view input);
  EngineResult ParseConversion(std::string_view input);
};

}  // namespace AXIOM
