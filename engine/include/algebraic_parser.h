// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once
#include "IParser.h"
#include "ast_nodes.h"
#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <functional>
#include <shared_mutex>
#include <optional>
#include <unordered_map>

#include "axiom_export.h"

namespace AXIOM {

using AXIOM::StringUnorderedMap;
using AXIOM::StringMap;

class AXIOM_EXPORT AlgebraicParser : public IParser {
public:
    AlgebraicParser();
    EngineResult ParseAndExecute(std::string_view input) override;
    EngineResult ParseAndExecuteWithContext(std::string_view input, const SymbolTable& context);
    
    NodePtr ParseExpression(std::string_view input);
    Arena& GetArena() { return arena_; }

private:
    Arena arena_;
    mutable std::shared_mutex mutex_s;
    mutable FixedVector<std::pair<std::string_view, EvalResult>, 1024> eval_cache_;
    static constexpr size_t MAX_CACHE_SIZE = 1000;

    struct CommandEntry { std::string_view command; std::function<EngineResult(std::string_view)> handler; };
    FixedVector<CommandEntry, 16> special_commands_;

    void RegisterSpecialCommands();
    EngineResult HandleQuadratic(std::string_view input);
    EngineResult HandleNonLinearSolve(std::string_view input);
    EngineResult HandleDerivative(std::string_view input);
    EngineResult HandleLimit(std::string_view input);
    EngineResult HandleIntegrate(std::string_view input);
    EngineResult HandlePlotFunction(std::string_view input);
    EngineResult SolveQuadratic(double a, double b, double c);
    EngineResult SolveNonLinearSystem(const FixedVector<std::string_view, 256>& equations, SymbolTable& guess);
};

} // namespace AXIOM


