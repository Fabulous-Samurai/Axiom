/**
 * @file dynamic_calc.h
 * @brief Optimized DynamicCalc using runtime polymorphism via IParser interface.
 */

#pragma once

#include "dynamic_calc_types.h"
#include "IParser.h"
#include "cpu_optimization.h"
#include <array>
#include <cstddef>
#include <memory>
#include <variant>

#include "algebraic_parser.h"
#include "linear_system_parser.h"
#include "statistics_parser.h"
#include "symbolic_parser.h"
#include "unit_parser.h"
#include "plot_parser.h"
#include "python_parser.h"

// Engine classes are declared in global namespace in this codebase.
class SymbolicEngine;
class StatisticsEngine;
class UnitManager;

#include "axiom_export.h"

namespace AXIOM
{

    enum class FastArithmeticOp
    {
        Add,
        Subtract,
        Multiply,
        Divide
    };

    using ParserVariant = std::variant<
        std::monostate,
        AXIOM::AlgebraicParser,
        AXIOM::LinearSystemParser,
        AXIOM::StatisticsParser,
        AXIOM::SymbolicParser,
        AXIOM::UnitParser,
        AXIOM::PlotParser,
        AXIOM::PythonParser
    >;

    class AXIOM_EXPORT DynamicCalc
    {
    private:
        static constexpr std::size_t kModeSlots = 7;
        std::array<ParserVariant, kModeSlots> parsers_{};
        CalculationMode current_mode_ = CalculationMode::ALGEBRAIC;

        std::unique_ptr<SymbolicEngine> symbolic_engine_;
        std::unique_ptr<StatisticsEngine> statistics_engine_;
        std::unique_ptr<UnitManager> unit_manager_;

        [[nodiscard]] static std::size_t ModeToIndex(CalculationMode mode) noexcept;

    public:
        DynamicCalc();
        ~DynamicCalc();

        EngineResult Evaluate(const std::string &input);
        [[nodiscard]] EngineResult EvaluateFast(double lhs, double rhs, FastArithmeticOp op) noexcept;

        /**
         * @brief Zero-overhead typed arithmetic dispatch.
         * Inlined across translation units for HPC/HFT hot-path performance.
         */
        [[nodiscard]] AXIOM_FORCE_INLINE static bool TryEvaluateFast(
            double lhs, double rhs, FastArithmeticOp op, double &out) noexcept
        {
            switch (op)
            {
            case FastArithmeticOp::Add:
                out = lhs + rhs;
                return true;
            case FastArithmeticOp::Subtract:
                out = lhs - rhs;
                return true;
            case FastArithmeticOp::Multiply:
                out = lhs * rhs;
                return true;
            case FastArithmeticOp::Divide:
                if (rhs == 0.0) [[unlikely]]
                {
                    return false;
                }
                out = lhs / rhs;
                return true;
            default:
                return false;
            }
        }
        void SetMode(CalculationMode mode) noexcept;
    };

} // namespace AXIOM



