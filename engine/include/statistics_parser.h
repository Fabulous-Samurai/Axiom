// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once
#include "IParser.h"
#include "statistics_engine.h"
#include "dynamic_calc_types.h"
#include <string>
#include <vector>

namespace AXIOM {

class StatisticsParser : public IParser {
public:
    explicit StatisticsParser(StatisticsEngine* engine) : engine_(engine) {}
    EngineResult ParseAndExecute(std::string_view input) noexcept override;
private:
    StatisticsEngine* engine_;
    static AXIOM::FixedVector<double, 256> ParseVector(const std::string& s);
};

} // namespace AXIOM
