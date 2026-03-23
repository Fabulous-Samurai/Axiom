// AXIOM Engine v3.0 - Edge Case & Stress Test Suite
// Tests boundary conditions, error handling, and extreme inputs

#include <iostream>
#include <vector>
#include <limits>
#include <cmath>
#include <format>

#include "../include/algebraic_parser.h"
#include "../include/statistics_engine.h"
#include "../include/linear_system_parser.h"
#include "../include/symbolic_engine.h"
#include "../include/unit_manager.h"
#include <string_view>

using namespace AXIOM;

using AXIOM::AlgebraicParser;
using AXIOM::LinearSystemParser;

class EdgeCaseRunner {
public:
    EdgeCaseRunner() = default;

    template<typename Func>
    void RunTest(std::string_view name, Func test_func) {
        total_tests_++;
        std::cout << std::format("  [TEST] {} ... ", name);
        try {
            if (test_func()) {
                std::cout << "\033[32mPASS\033[0m\n";
                passed_tests_++;
            } else {
                std::cout << "\033[31mFAIL\033[0m (assertion failed)\n";
            }
        } catch (const std::runtime_error& e) {
            std::cout << std::format("\033[31mFAIL\033[0m (runtime error: {})\n", e.what());
        } catch (const std::exception& e) {
            std::cout << std::format("\033[31mFAIL\033[0m (exception: {})\n", e.what());
        } catch (...) {
            std::cout << "\033[31mFAIL\033[0m (unknown error)\n";
        }
    }

    void StartSection(std::string_view name) {
        std::cout << std::format("\n========================================\n  {}\n========================================\n", name);
    }

    void EndSection() {
        // Reserved for future cleanup or logging
    }

    void Log(std::string_view msg) {
        std::cout << msg << "\n";
    }

    void PrintSummary() {
        std::cout << std::format(
            "\n========================================\n"
            "           TEST SUMMARY\n"
            "========================================\n"
            "Total:   {}\n"
            "Passed:  {}\n"
            "Failed:  {}\n",
            total_tests_, passed_tests_, total_tests_ - passed_tests_);

        if (passed_tests_ == total_tests_) {
            std::cout << "\n\033[32m✓ ALL EDGE CASE TESTS PASSED!\033[0m\n\n";
        } else {
            std::cout << "\n\033[31m✗ SOME EDGE CASE TESTS FAILED!\033[0m\n\n";
        }
    }

    int GetFailedCount() const { return total_tests_ - passed_tests_; }

private:
    int total_tests_{0};
    int passed_tests_{0};
};

void TestAlgebraicParserBasic(EdgeCaseRunner& runner, AlgebraicParser& parser) {
    // Empty input
    runner.RunTest("Empty string input", [&parser]() {
        auto result = parser.ParseAndExecute("");
        return !result.HasResult(); // Should fail gracefully
    });

    // Very long expression
    runner.RunTest("Very long expression chain", [&parser]() {
        std::string expr = "1";
        for (int i = 0; i < 100; ++i) {
            expr += "+1";
        }
        auto result = parser.ParseAndExecute(expr);
        return result.HasResult() && std::abs(*result.GetDouble() - 101.0) < 0.01;
    });

    // Division by zero
    runner.RunTest("Division by zero", [&parser]() {
        auto result = parser.ParseAndExecute("10/0");
        return !result.HasResult() || std::isinf(*result.GetDouble());
    });
}

void TestAlgebraicParserLimits(EdgeCaseRunner& runner, AlgebraicParser& parser) {
    // Very small numbers
    runner.RunTest("Very small number (1e-100)", [&parser]() {
        auto result = parser.ParseAndExecute("1e-100");
        return result.HasResult();
    });

    // Very large numbers
    runner.RunTest("Very large number (1e100)", [&parser]() {
        auto result = parser.ParseAndExecute("1e100");
        return result.HasResult();
    });

    // Negative zero
    runner.RunTest("Negative zero arithmetic", [&parser]() {
        auto result = parser.ParseAndExecute("-0 + 0");
        return result.HasResult() && std::abs(*result.GetDouble()) < 1e-10;
    });
}

void TestAlgebraicParserNesting(EdgeCaseRunner& runner, AlgebraicParser& parser) {
    // Nested parentheses (deep)
    runner.RunTest("Deeply nested parentheses (10 levels)", [&parser]() {
        std::string expr = "((((((((((1+1))))))))))";
        auto result = parser.ParseAndExecute(expr);
        return result.HasResult() && std::abs(*result.GetDouble() - 2.0) < 0.01;
    });

    // Enormous complex expression (Sandbox check)
    runner.RunTest("Enormous complex expression (Stress Test)", [&parser]() {
        std::string expr = "sin(cos(tan(log(sqrt(144)))))";
        for (int i = 0; i < 50; ++i) {
            expr += std::format(" + {} * sin({})", i, i);
        }
        auto result = parser.ParseAndExecute(expr);
        return result.HasResult();
    });
}

void TestAlgebraicParserErrorHandling(EdgeCaseRunner& runner, AlgebraicParser& parser) {
    // Missing operand
    runner.RunTest("Missing operand (5+)", [&parser]() {
        auto result = parser.ParseAndExecute("5+");
        return !result.HasResult(); // Should fail
    });

    // Unknown function
    runner.RunTest("Unknown function (foo(5))", [&parser]() {
        auto result = parser.ParseAndExecute("foo(5)");
        return !result.HasResult(); // Should fail
    });

    // Undefined variable
    runner.RunTest("Undefined variable (x+5)", [&parser]() {
        auto result = parser.ParseAndExecute("x+5");
        return !result.HasResult(); // Should fail without context
    });

    // Special characters
    runner.RunTest("Special characters (!@#$%)", [&parser]() {
        auto result = parser.ParseAndExecute("5!@#$%3");
        return !result.HasResult(); // Should fail
    });

    // NaN operations
    runner.RunTest("sqrt of negative number", [&parser]() {
        auto result = parser.ParseAndExecute("sqrt(-1)");
        return result.HasResult();
    });
}

void TestAlgebraicParserEdgeCases(EdgeCaseRunner& runner) {
    runner.StartSection("ALGEBRAIC PARSER EDGE CASES");
    
    AlgebraicParser parser;
    TestAlgebraicParserBasic(runner, parser);
    TestAlgebraicParserLimits(runner, parser);
    TestAlgebraicParserNesting(runner, parser);
    TestAlgebraicParserErrorHandling(runner, parser);

    runner.EndSection();
}

void TestStatisticsEngineBase(EdgeCaseRunner& runner, StatisticsEngine& stats) {
    // Empty vector
    runner.RunTest("Mean of empty vector", [&stats]() {
        std::vector<double> empty;
        auto result = stats.Mean(empty);
        return !result.HasResult(); // Should fail
    });

    // Single element
    runner.RunTest("Mean of single element", [&stats]() {
        std::vector<double> single = {5.0};
        auto result = stats.Mean(single);
        return result.HasResult() && std::abs(*result.GetDouble() - 5.0) < 0.01;
    });

    // All same values
    runner.RunTest("Variance of constant values", [&stats]() {
        std::vector<double> constant(100, 5.0);
        auto result = stats.Variance(constant);
        return result.HasResult() && std::abs(*result.GetDouble()) < 0.01; // Should be 0
    });
}

void TestStatisticsEngineAdvanced(EdgeCaseRunner& runner, StatisticsEngine& stats) {
    // Extreme values
    runner.RunTest("Mean with very large values", [&stats]() {
        std::vector<double> large = {1e100, 1e100, 1e100};
        auto result = stats.Mean(large);
        return result.HasResult() && *result.GetDouble() > 1e99;
    });

    // Negative values
    runner.RunTest("Mean of all negative values", [&stats]() {
        std::vector<double> negative = {-1.0, -2.0, -3.0, -4.0, -5.0};
        auto result = stats.Mean(negative);
        return result.HasResult() && std::abs(*result.GetDouble() + 3.0) < 0.01;
    });

    // Mixed positive/negative
    runner.RunTest("Variance with mixed signs", [&stats]() {
        std::vector<double> mixed = {-10.0, -5.0, 0.0, 5.0, 10.0};
        auto result = stats.Variance(mixed);
        return result.HasResult() && *result.GetDouble() > 0;
    });
}

void TestStatisticsEngineCorrelation(EdgeCaseRunner& runner, StatisticsEngine& stats) {
    // Correlation with mismatched sizes
    runner.RunTest("Correlation with different vector sizes", [&stats]() {
        std::vector<double> x = {1.0, 2.0, 3.0};
        std::vector<double> y = {1.0, 2.0};
        auto result = stats.Correlation(x, y);
        return !result.HasResult(); // Should fail
    });

    // Perfect correlation
    runner.RunTest("Perfect positive correlation", [&stats]() {
        std::vector<double> x = {1.0, 2.0, 3.0, 4.0, 5.0};
        std::vector<double> y = {2.0, 4.0, 6.0, 8.0, 10.0}; // y = 2x
        auto result = stats.Correlation(x, y);
        return result.HasResult() && std::abs(*result.GetDouble() - 1.0) < 0.01;
    });

    // No correlation
    runner.RunTest("Zero correlation (uncorrelated)", [&stats]() {
        std::vector<double> x = {1.0, 2.0, 3.0, 4.0, 5.0};
        std::vector<double> y = {5.0, 3.0, 5.0, 3.0, 5.0}; // constant alternating
        auto result = stats.Correlation(x, y);
        return result.HasResult(); // Should succeed with near-zero correlation
    });
}

void TestStatisticsEnginePercentiles(EdgeCaseRunner& runner, StatisticsEngine& stats) {
    // Percentile edge cases
    runner.RunTest("0th percentile", [&stats]() {
        std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
        auto result = stats.Percentile(data, 0.0);
        return result.HasResult() && std::abs(*result.GetDouble() - 1.0) < 0.01;
    });

    runner.RunTest("100th percentile", [&stats]() {
        std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
        auto result = stats.Percentile(data, 100.0);
        return result.HasResult() && std::abs(*result.GetDouble() - 5.0) < 0.01;
    });

    runner.RunTest("Invalid percentile (150)", [&stats]() {
        std::vector<double> data = {1.0, 2.0, 3.0};
        auto result = stats.Percentile(data, 150.0);
        return !result.HasResult(); // Should fail
    });

    // Linear regression with insufficient data
    runner.RunTest("Linear regression with 1 point", [&stats]() {
        std::vector<double> x = {1.0};
        std::vector<double> y = {2.0};
        auto result = stats.LinearRegression(x, y);
        return !result.HasResult(); // Should fail (need at least 2 points)
    });
}

void TestStatisticsEngineEdgeCases(EdgeCaseRunner& runner) {
    runner.StartSection("STATISTICS ENGINE EDGE CASES");
    
    StatisticsEngine stats;

    TestStatisticsEngineBase(runner, stats);
    TestStatisticsEngineAdvanced(runner, stats);
    TestStatisticsEngineCorrelation(runner, stats);
    TestStatisticsEnginePercentiles(runner, stats);

    runner.EndSection();
}

void TestLinearSystemParserBasic(EdgeCaseRunner& runner, LinearSystemParser& parser) {
    // Singular matrix (no unique solution)
    runner.RunTest("Singular matrix (determinant = 0)", [&parser]() {
        auto result = parser.ParseAndExecute("solve [[1,2],[2,4]] [3,6]");
        return !result.HasResult();
    });

    // 1x1 system
    runner.RunTest("1x1 system (trivial)", [&parser]() {
        auto result = parser.ParseAndExecute("solve [[5]] [10]");
        return result.HasResult();
    });
}

void TestLinearSystemParserDimensions(EdgeCaseRunner& runner, LinearSystemParser& parser) {
    // Overdetermined system
    runner.RunTest("Empty matrix", [&parser]() {
        auto result = parser.ParseAndExecute("solve [[]] []");
        return !result.HasResult();
    });

    // Mismatched dimensions
    runner.RunTest("Matrix-vector dimension mismatch", [&parser]() {
        auto result = parser.ParseAndExecute("solve [[1,2],[3,4]] [5]");
        return !result.HasResult();
    });

    // Non-square matrix
    runner.RunTest("Non-square matrix (3x2)", [&parser]() {
        auto result = parser.ParseAndExecute("solve [[1,2],[3,4],[5,6]] [7,8,9]");
        return !result.HasResult();
    });
}

void TestLinearSystemParserNumerical(EdgeCaseRunner& runner, LinearSystemParser& parser) {
    // Very large coefficients
    runner.RunTest("Large coefficients", [&parser]() {
        auto result = parser.ParseAndExecute("solve [[1000000,2000000],[3000000,4000000]] [5000000,11000000]");
        return result.HasResult();
    });

    // Very small coefficients
    runner.RunTest("Small coefficients (near zero)", [&parser]() {
        auto result = parser.ParseAndExecute("solve [[0.0001,0.0002],[0.0003,0.0004]] [0.0005,0.0011]");
        return result.HasResult();
    });

    // Malformed input
    runner.RunTest("Malformed matrix notation", [&parser]() {
        auto result = parser.ParseAndExecute("solve [1,2],[3,4 [5,6]");
        return !result.HasResult();
    });
}

void TestLinearSystemParserEdgeCases(EdgeCaseRunner& runner) {
    runner.StartSection("LINEAR SYSTEM PARSER EDGE CASES");
    
    LinearSystemParser parser;
    TestLinearSystemParserBasic(runner, parser);
    TestLinearSystemParserDimensions(runner, parser);
    TestLinearSystemParserNumerical(runner, parser);

    runner.EndSection();
}

void TestSymbolicEngineBasic(EdgeCaseRunner& runner, SymbolicEngine& symbolic) {
    // Empty expression
    runner.RunTest("Empty expression", [&symbolic]() {
        auto result = symbolic.Expand("");
        return !result.HasResult();
    });

    // Constant expression
    runner.RunTest("Expand constant (5)", [&symbolic]() {
        auto result = symbolic.Expand("5");
        return result.HasResult();
    });

    // Very complex polynomial
    runner.RunTest("Expand (x+1)^10", [&symbolic]() {
        auto result = symbolic.Expand("(x+1)^10");
        return result.HasResult();
    });
}

void TestSymbolicEngineCalculus(EdgeCaseRunner& runner, SymbolicEngine& symbolic) {
    // Integration of constant
    runner.RunTest("Integrate constant", [&symbolic]() {
        auto result = symbolic.Integrate("5", "x");
        return result.HasResult();
    });

    // Differentiation of constant
    runner.RunTest("Differentiate constant", [&symbolic]() {
        auto result = symbolic.Differentiate("5", "x");
        return result.HasResult();
    });

    // Nested functions
    runner.RunTest("Differentiate sin(cos(x))", [&symbolic]() {
        auto result = symbolic.Differentiate("sin(cos(x))", "x");
        return result.HasResult();
    });
}

void TestSymbolicEngineManipulation(EdgeCaseRunner& runner, SymbolicEngine& symbolic) {
    // Undefined variable in substitution
    runner.RunTest("Substitute undefined variable", [&symbolic]() {
        auto result = symbolic.Substitute("x+y", "z", "5");
        return result.HasResult();
    });

    // Factor prime number
    runner.RunTest("Factor prime expression", [&symbolic]() {
        auto result = symbolic.Factor("x^2 + x + 1");
        return result.HasResult();
    });
}

void TestSymbolicEngineEdgeCases(EdgeCaseRunner& runner) {
    runner.StartSection("SYMBOLIC ENGINE EDGE CASES");
    
    SymbolicEngine symbolic;
    TestSymbolicEngineBasic(runner, symbolic);
    TestSymbolicEngineCalculus(runner, symbolic);
    TestSymbolicEngineManipulation(runner, symbolic);

    runner.EndSection();
}

void TestUnitManagerBasic(EdgeCaseRunner& runner, UnitManager& units) {
    // Zero conversion
    runner.RunTest("Convert 0 km to m", [&units]() {
        auto result = units.ConvertUnit(0.0, "km", "m");
        return result.HasResult() && std::abs(*result.GetDouble()) < 0.01;
    });

    // Negative values
    runner.RunTest("Convert -5 m to cm", [&units]() {
        auto result = units.ConvertUnit(-5.0, "m", "cm");
        return result.HasResult() && std::abs(*result.GetDouble() + 500.0) < 0.1;
    });

    // Same unit conversion
    runner.RunTest("Convert m to m (identity)", [&units]() {
        auto result = units.ConvertUnit(5.0, "m", "m");
        return result.HasResult() && std::abs(*result.GetDouble() - 5.0) < 0.01;
    });
}

void TestUnitManagerErrorHandling(EdgeCaseRunner& runner, UnitManager& units) {
    // Unknown source unit
    runner.RunTest("Convert from unknown unit", [&units]() {
        auto result = units.ConvertUnit(5.0, "foobar", "m");
        return !result.HasResult();
    });

    // Unknown target unit
    runner.RunTest("Convert to unknown unit", [&units]() {
        auto result = units.ConvertUnit(5.0, "m", "foobar");
        return !result.HasResult();
    });

    // Incompatible units
    runner.RunTest("Incompatible units (kg to m)", [&units]() {
        return !units.AreCompatible("kg", "m");
    });
}

void TestUnitManagerEdgeCases(EdgeCaseRunner& runner) {
    runner.StartSection("UNIT MANAGER EDGE CASES");

    UnitManager units;
    TestUnitManagerBasic(runner, units);
    TestUnitManagerErrorHandling(runner, units);

    runner.EndSection();
}

int main() {
    std::cout << "\n╔═══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                                                               ║" << std::endl;
    std::cout << "║        AXIOM ENGINE v3.0 - EDGE CASE TEST SUITE               ║" << std::endl;
    std::cout << "║                                                               ║" << std::endl;
    std::cout << "║         Boundary Conditions & Error Handling                  ║" << std::endl;
    std::cout << "║                                                               ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════════════╝" << std::endl;

    EdgeCaseRunner runner;

    TestAlgebraicParserEdgeCases(runner);
    TestStatisticsEngineEdgeCases(runner);
    TestLinearSystemParserEdgeCases(runner);
    TestSymbolicEngineEdgeCases(runner);
    TestUnitManagerEdgeCases(runner);

    runner.PrintSummary();

    return runner.GetFailedCount();
}

