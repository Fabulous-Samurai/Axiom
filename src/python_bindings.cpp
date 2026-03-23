/**
 * @file python_bindings.cpp
 * @brief Python FFI layer for AXIOM Zenith using nanobind
 */

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include "dynamic_calc.h"
#include "arena_allocator.h"
#include "telemetry.h"

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(axiom_core, m) {
    m.doc() = "AXIOM Zenith: Principal Performance Compute Engine";

    // 1. DynamicCalc Interface
    nb::class_<AXIOM::DynamicCalc>(m, "DynamicCalc")
        .def(nb::init<>())
        .def("evaluate", [](AXIOM::DynamicCalc& self, const std::string& input) {
            auto result = self.Evaluate(input);
            if (result.HasResult()) {
                if (result.GetDouble().has_value()) {
                    return nb::cast(result.GetDouble().value());
                } else if (result.GetComplex().has_value()) {
                    return nb::cast(result.GetComplex().value());
                }
            }
            return nb::cast<nb::object>(nb::none());
        }, "input"_a, "Evaluates an expression with nanosecond precision.")
        .def("evaluate_fast", [](AXIOM::DynamicCalc& self, double lhs, double rhs, AXIOM::FastArithmeticOp op) {
            auto result = self.EvaluateFast(lhs, rhs, op);
            if (result.HasResult()) {
                if (result.GetDouble().has_value()) {
                    return nb::cast(result.GetDouble().value());
                }
            }
            return nb::cast<nb::object>(nb::none());
        }, "lhs"_a, "rhs"_a, "op"_a, "High-velocity arithmetic dispatch path.");

    // 2. Telemetry Control
    m.def("start_telemetry", []() {
        AXIOM::TelemetryScribe::instance().start("telemetry.log");
    }, "Initializes the zero-observer telemetry scribe.");

    // 3. CalculationMode & Operation Enums
    nb::enum_<AXIOM::CalculationMode>(m, "CalculationMode")
        .value("ALGEBRAIC", AXIOM::CalculationMode::ALGEBRAIC)
        .value("LINEAR_SYSTEM", AXIOM::CalculationMode::LINEAR_SYSTEM)
        .export_values();

    nb::enum_<AXIOM::FastArithmeticOp>(m, "FastArithmeticOp")
        .value("ADD", AXIOM::FastArithmeticOp::Add)
        .value("SUBTRACT", AXIOM::FastArithmeticOp::Subtract)
        .value("MULTIPLY", AXIOM::FastArithmeticOp::Multiply)
        .value("DIVIDE", AXIOM::FastArithmeticOp::Divide)
        .export_values();
}

