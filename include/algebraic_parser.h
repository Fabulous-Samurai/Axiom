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
    EngineResult ParseAndExecute(const std::string& input) override;
    EngineResult ParseAndExecuteWithContext(const std::string& input, const StringUnorderedMap<Number>& context);
    
    EngineResult ParseAndExecuteWithContext(const std::string& input, const StringMap<Number>& context) {
        StringUnorderedMap<Number> fast_context;
        fast_context.reserve(context.size());
        for (const auto& [key, value] : context) fast_context.emplace(key, value);
        return ParseAndExecuteWithContext(input, fast_context);
    }

        EngineResult ParseAndExecuteWithContext(const std::string& input, const StringMap<double>& context) {
        StringUnorderedMap<Number> number_context;
        number_context.reserve(context.size());
        for (const auto& [key, value] : context) number_context.emplace(key, Number(value));
        return ParseAndExecuteWithContext(input, number_context);
    }

    EngineResult ParseAndExecuteWithContext(const std::string& input, const StringUnorderedMap<double>& context) {
        StringUnorderedMap<Number> number_context;
        number_context.reserve(context.size());
        for (const auto& [key, value] : context) number_context.emplace(key, Number(value));
        return ParseAndExecuteWithContext(input, number_context);
    }

    NodePtr ParseExpression(std::string_view input);
    Arena& GetArena() { return arena_; }

private:
    Arena arena_;
    mutable std::shared_mutex mutex_s;
    mutable StringUnorderedMap<EvalResult> eval_cache_;
    static constexpr size_t MAX_CACHE_SIZE = 1000;

    struct CommandEntry { std::string command; std::function<EngineResult(const std::string&)> handler; };
    std::vector<CommandEntry> special_commands_;

    void RegisterSpecialCommands();
    EngineResult HandleQuadratic(const std::string& input);
    EngineResult HandleNonLinearSolve(const ::std::string& input);
    EngineResult HandleDerivative(const ::std::string& input);
    EngineResult HandleLimit(const ::std::string& input);
    EngineResult HandleIntegrate(const ::std::string& input);
    EngineResult HandlePlotFunction(const ::std::string& input);
    EngineResult SolveQuadratic(double a, double b, double c);
    EngineResult SolveNonLinearSystem(const std::vector<std::string>& equations, StringUnorderedMap<double>& guess);
};

} // namespace AXIOM


