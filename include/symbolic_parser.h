#pragma once
#include "iParser.h"

class SymbolicEngine;

namespace AXIOM {

class SymbolicParser : public IParser {
public:
    explicit SymbolicParser(SymbolicEngine* engine) noexcept : engine_(engine) {}
    EngineResult ParseAndExecute(const std::string& input) override;
private:
    SymbolicEngine* engine_;
};

} // namespace AXIOM

