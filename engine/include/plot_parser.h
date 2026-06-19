// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#pragma once

#include <string>

#include "IParser.h"
#include "plot_engine.h"

namespace AXIOM {

class PlotParser : public IParser {
 public:
  PlotParser() = default;
  EngineResult ParseAndExecute(std::string_view input) noexcept override;

 private:
  PlotEngine plot_engine_;
};

}  // namespace AXIOM
