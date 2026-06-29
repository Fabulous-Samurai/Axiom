// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file dynamic_calc.cpp
 * @brief Implementation of Static Dispatch Logic
 */

#include "dynamic_calc.h"
#include "algebraic_parser.h"
#include "expression_policy.h"
#include "linear_system_parser.h"
#include "statistics_parser.h"
#include "statistics_engine.h"
#include "symbolic_parser.h"
#include "symbolic_engine.h"
#include "unit_parser.h"
#include "unit_manager.h"
#include "plot_parser.h"
#include "python_parser.h"
#include "python_engine.h"
#include "telemetry_base.h"
#include <memory>

namespace {
constexpr std::size_t kInvalidModeIndex = static_cast<std::size_t>(-1);
}

namespace AXIOM {

std::size_t DynamicCalc::ModeToIndex(CalculationMode mode) noexcept {
    const auto raw = static_cast<std::size_t>(mode);
    return raw < kModeSlots ? raw : kInvalidModeIndex;
}

DynamicCalc::DynamicCalc() {
    statistics_engine_ = std::make_unique<StatisticsEngine>();
    symbolic_engine_   = std::make_unique<SymbolicEngine>();
    unit_manager_      = std::make_unique<UnitManager>();

    parsers_[ModeToIndex(CalculationMode::ALGEBRAIC)].emplace<AXIOM::AlgebraicParser>();
    parsers_[ModeToIndex(CalculationMode::LINEAR_SYSTEM)].emplace<AXIOM::LinearSystemParser>();
    parsers_[ModeToIndex(CalculationMode::STATISTICS)].emplace<AXIOM::StatisticsParser>(statistics_engine_.get());
    parsers_[ModeToIndex(CalculationMode::SYMBOLIC)].emplace<AXIOM::SymbolicParser>(symbolic_engine_.get());
    parsers_[ModeToIndex(CalculationMode::UNITS)].emplace<AXIOM::UnitParser>(unit_manager_.get());
    parsers_[ModeToIndex(CalculationMode::PLOT)].emplace<AXIOM::PlotParser>();
    parsers_[ModeToIndex(CalculationMode::PYTHON)].emplace<AXIOM::PythonParser>();
}

DynamicCalc::~DynamicCalc() = default;

EngineResult DynamicCalc::Evaluate(std::string_view input) noexcept {
    const auto policy = AssessExpressionPolicy(input, current_mode_);
    if (!policy.allowed) return CreateErrorResult(policy.error);

    const std::size_t idx = ModeToIndex(current_mode_);
    if (idx == kInvalidModeIndex) return CreateErrorResult(CalcErr::OperationNotFound);

    return std::visit([input](auto& parser) -> EngineResult {
        using T = std::decay_t<decltype(parser)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            return CreateErrorResult(CalcErr::OperationNotFound);
        } else {
            return parser.ParseAndExecute(input);
        }
    }, parsers_[idx]);
}

EngineResult DynamicCalc::EvaluateFast(double lhs, double rhs, FastArithmeticOp op) noexcept {
    double out = 0.0;
    if (!TryEvaluateFast(lhs, rhs, op, out)) {
        if (op == FastArithmeticOp::Divide && rhs == 0.0) return CreateErrorResult(CalcErr::DivideByZero);
        return CreateErrorResult(CalcErr::OperationNotFound);
    }
    return CreateSuccessResult(out);
}

void DynamicCalc::SetMode(CalculationMode mode) noexcept {
    current_mode_ = mode;
}

} // namespace AXIOM
