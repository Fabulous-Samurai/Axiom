// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once

#include "IParser.h"
#include "plot_engine.h"
#include <string>

namespace AXIOM {

class PlotParser : public IParser {
public:
    PlotParser() = default;
    EngineResult ParseAndExecute(const std::string& input) override;

private:
    PlotEngine plot_engine_;
};

} // namespace AXIOM

