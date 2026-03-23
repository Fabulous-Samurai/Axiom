/**
 * @file symbolic_engine.h  
 * @brief Advanced symbolic computation capabilities
 * Adds algebraic manipulation, equation solving, and symbolic integration
 */
#pragma once

#include "algebraic_parser.h"
#include <unordered_set>

#include "axiom_export.h"

class AXIOM_EXPORT SymbolicEngine {
public:
    // Advanced algebraic operations
    AXIOM::EngineResult Expand(const std::string& expression);         // (x+1)^2 -> x^2 + 2x + 1
    AXIOM::EngineResult Factor(const std::string& expression);         // x^2 - 1 -> (x-1)(x+1)
    AXIOM::EngineResult Simplify(const std::string& expression);       // Complex simplification
    AXIOM::EngineResult Substitute(const std::string& expr, const std::string& var, const std::string& value);
    
    // Calculus operations  
    AXIOM::EngineResult Integrate(const std::string& expression, const std::string& variable);
    AXIOM::EngineResult DefiniteIntegral(const std::string& expr, const std::string& var, double a, double b);
    AXIOM::EngineResult PartialDerivative(const std::string& expr, const std::string& var);
    AXIOM::EngineResult Differentiate(const std::string& expr, const std::string& var) { return PartialDerivative(expr, var); }
    AXIOM::EngineResult TaylorSeries(const std::string& expr, const std::string& var, double point, int order);
    
    // Equation solving
    AXIOM::EngineResult SolveEquation(const std::string& equation, const std::string& variable);
    AXIOM::EngineResult SolveSystem(const std::vector<std::string>& equations, const std::vector<std::string>& variables);
    
    // Advanced features
    AXIOM::EngineResult FindLimits(const std::string& expr, const std::string& var, double approach_point);
    AXIOM::EngineResult FindRoots(const std::string& expr, const std::string& var, double range_min, double range_max);
};
