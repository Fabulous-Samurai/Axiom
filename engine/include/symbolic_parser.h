// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#pragma once
#include "IParser.h"

namespace AXIOM {

class SymbolicEngine;

class SymbolicParser : public IParser {
 public:
  explicit SymbolicParser(SymbolicEngine* engine) noexcept : engine_(engine) {}
  EngineResult ParseAndExecute(std::string_view input) noexcept override;

 private:
  SymbolicEngine* engine_;
};

}  // namespace AXIOM
