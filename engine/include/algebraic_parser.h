// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once
#include "IParser.h"
#include "ast_nodes.h"
#include <string_view>
#include <memory>
#include <functional>
#include <shared_mutex>
#include <optional>

#include "axiom_export.h"

namespace AXIOM {

using AXIOM::StringUnorderedMap;
using AXIOM::StringMap;

class AXIOM_EXPORT AlgebraicParser : public IParser {
public:
    EngineResult ParseAndExecute(std::string_view input) noexcept override;
    EngineResult ParseAndExecuteWithContext(std::string_view input, const SymbolTable& context) noexcept;

    
    NodePtr ParseExpression(std::string_view input) noexcept;
    Arena& GetArena() noexcept { return arena_; }

private:
    Arena arena_;
    mutable std::shared_mutex mutex_s;
    mutable FixedVector<std::pair<std::string_view, EvalResult>, 1024> eval_cache_;
    static constexpr size_t MAX_CACHE_SIZE = 1000;

    struct CommandEntry { std::string_view command; std::function<EngineResult(std::string_view)> handler; };
    FixedVector<CommandEntry, 16> special_commands_;

    void RegisterSpecialCommands() noexcept;
    EngineResult HandleQuadratic(std::string_view input) noexcept;
    EngineResult HandleNonLinearSolve(std::string_view input) noexcept;
    EngineResult HandleDerivative(std::string_view input) noexcept;
    EngineResult HandleLimit(std::string_view input) noexcept;
    EngineResult HandleIntegrate(std::string_view input) noexcept;
    EngineResult HandlePlotFunction(std::string_view input) noexcept;
    EngineResult SolveQuadratic(double a, double b, double c) noexcept;
    EngineResult SolveNonLinearSystem(const FixedVector<std::string_view, 256>& equations, SymbolTable& guess) noexcept;
};

} // namespace AXIOM


