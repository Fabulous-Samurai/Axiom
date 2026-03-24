// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once

#ifndef CPP_DYNAMIC_CALC_IPARSER_H
#define CPP_DYNAMIC_CALC_IPARSER_H

#include <string>
#include "dynamic_calc_types.h"

#include "axiom_export.h"

namespace AXIOM {

class AXIOM_EXPORT IParser {
public:
    virtual ~IParser() = default;
    virtual EngineResult ParseAndExecute(const std::string& input) = 0;
};

} // namespace AXIOM

#endif //CPP_DYNAMIC_CALC_IPARSER_H
