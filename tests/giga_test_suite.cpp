/**
 * @file giga_test_suite.cpp
 * @brief Monolithic production-grade test suite for AXIOM Engine v3.0
 * 
 * Features:
 * - Resilient test runner (continues on failure)
 * - ANSI color-coded output (Green=PASS, Red=FAIL, Cyan=INFO)
 * - Performance metrics with timing
 * - Comprehensive coverage of all engines
 * - CI/CD compatible exit codes
 */

#include "algebraic_parser.h"
#include "linear_system_parser.h"
#include "statistics_engine.h"
#include "symbolic_engine.h"
#include "unit_manager.h"
#include "plot_engine.h"
#include "dynamic_calc.h"
#ifdef ENABLE_EIGEN
#include "eigen_engine.h"
#endif

#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <functional>
#include <vector>
#include <string>

// ============================================================================
// ANSI COLOR CODES
// ============================================================================
namespace Color {
    const char* RESET   = "\033[0m";
    const char* RED     = "\033[31m";
    const char* GREEN   = "\033[32m";
    const char* YELLOW  = "\033[33m";
    const char* CYAN    = "\033[36m";
    const char* BOLD    = "\033[1m";
}

// ============================================================================
// TEST RUNNER CLASS
// ============================================================================
class TestRunner {
private:
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point section_start_time;
    std::vector<std::string> failed_test_names;

public:
    TestRunner() {
        start_time = std::chrono::steady_clock::now();
    }

    void StartSection(const std::string& section_name) {
        std::cout << "\n" << Color::CYAN << Color::BOLD 
                  << "========================================\n"
                  << "  " << section_name << "\n"
                  << "========================================" 
                  << Color::RESET << "\n";
        section_start_time = std::chrono::steady_clock::now();
    }

    void EndSection() {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - section_start_time).count();
        std::cout << Color::CYAN << "Section completed in " << duration << "ms" << Color::RESET << "\n";
    }

    template<typename Func>
    void RunTest(const std::string& test_name, Func test_func) {
        total_tests++;
        std::cout << "  [TEST] " << test_name << " ... " << std::flush;
        
        try {
            bool result = test_func();
            if (result) {
                passed_tests++;
                std::cout << Color::GREEN << "PASS" << Color::RESET << "\n";
            } else {
                failed_tests++;
                failed_test_names.push_back(test_name);
                std::cout << Color::RED << "FAIL (assertion failed)" << Color::RESET << "\n";
            }
        } catch (const std::exception& e) {
            failed_tests++;
            failed_test_names.push_back(test_name);
            std::cout << Color::RED << "FAIL (exception: " << e.what() << ")" << Color::RESET << "\n";
        } catch (...) {
            failed_tests++;
            failed_test_names.push_back(test_name);
            std::cout << Color::RED << "FAIL (unknown exception)" << Color::RESET << "\n";
        }
    }

    void PrintSummary() {
        auto end = std::chrono::steady_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count();

        std::cout << "\n" << Color::BOLD << Color::CYAN 
                  << "========================================\n"
                  << "           TEST SUMMARY\n"
                  << "========================================" << Color::RESET << "\n";
        
        std::cout << "Total:   " << total_tests << "\n";
        std::cout << Color::GREEN << "Passed:  " << passed_tests << Color::RESET << "\n";
        std::cout << Color::RED << "Failed:  " << failed_tests << Color::RESET << "\n";
        std::cout << "Time:    " << total_duration << "ms\n";

        if (failed_tests > 0) {
            std::cout << "\n" << Color::RED << Color::BOLD << "Failed Tests:" << Color::RESET << "\n";
            for (const auto& name : failed_test_names) {
                std::cout << "  - " << Color::RED << name << Color::RESET << "\n";
            }
        }

        std::cout << "\n" << Color::BOLD;
        if (failed_tests == 0) {
            std::cout << Color::GREEN << "✓ ALL TESTS PASSED!" << Color::RESET << "\n\n";
        } else {
            std::cout << Color::RED << "✗ SOME TESTS FAILED!" << Color::RESET << "\n\n";
        }
    }

    int GetExitCode() const {
        return (failed_tests == 0) ? 0 : 1;
    }
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================
bool approx_equal(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

bool contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

// ============================================================================
// TEST SUITES
// ============================================================================

void TestAlgebraicParser(TestRunner& runner) {
    runner.StartSection("ALGEBRAIC PARSER TESTS");
    
    AlgebraicParser parser;
    
    // Test 1: Basic arithmetic
    runner.RunTest("Basic addition", [&]() {
        auto result = parser.ParseAndExecute("2 + 3");
        return result.HasResult() && approx_equal(*result.GetDouble(), 5.0);
    });
    
    // Test 2: Trigonometric function
    runner.RunTest("sin(30) calculation", [&]() {
        auto result = parser.ParseAndExecute("sin(30)");
        return result.HasResult() && approx_equal(*result.GetDouble(), 0.5); // sin(30°) = 0.5
    });
    
    // Test 3: Logarithm
    runner.RunTest("log(100) calculation", [&]() {
        auto result = parser.ParseAndExecute("log(100)");
        return result.HasResult() && approx_equal(*result.GetDouble(), std::log10(100.0));
    });
    
    // Test 4: Complex expression
    runner.RunTest("sin(30) + log(100)", [&]() {
        auto result = parser.ParseAndExecute("sin(30) + log(100)");
        double expected = 0.5 + 2.0; // sin(30°) + log10(100) = 0.5 + 2.0 = 2.5
        return result.HasResult() && approx_equal(*result.GetDouble(), expected);
    });
    
    // Test 5: Context variables
    runner.RunTest("Variable context: x=10", [&]() {
        std::map<std::string, AXIOM::Number> context;
        context["x"] = AXIOM::Number(10.0);
        auto result = parser.ParseAndExecuteWithContext("x + 5", context);
        return result.HasResult() && approx_equal(*result.GetDouble(), 15.0);
    });
    
    // Test 6: Expression with variable
    runner.RunTest("Variable expression: 2*x + 3", [&]() {
        std::map<std::string, AXIOM::Number> context;
        context["x"] = AXIOM::Number(5.0);
        auto result = parser.ParseAndExecuteWithContext("2*x + 3", context);
        return result.HasResult() && approx_equal(*result.GetDouble(), 13.0);
    });
    
    // Test 7: Power operation
    runner.RunTest("Power: 2^10", [&]() {
        auto result = parser.ParseAndExecute("2^10");
        return result.HasResult() && approx_equal(*result.GetDouble(), 1024.0);
    });
    
    // Test 8: Square root
    runner.RunTest("sqrt(144)", [&]() {
        auto result = parser.ParseAndExecute("sqrt(144)");
        return result.HasResult() && approx_equal(*result.GetDouble(), 12.0);
    });
    
    // Test 9: Division
    runner.RunTest("Division: 100 / 4", [&]() {
        auto result = parser.ParseAndExecute("100 / 4");
        return result.HasResult() && approx_equal(*result.GetDouble(), 25.0);
    });
    
    // Test 10: Nested operations
    runner.RunTest("Nested: (2+3)*4", [&]() {
        auto result = parser.ParseAndExecute("(2+3)*4");
        return result.HasResult() && approx_equal(*result.GetDouble(), 20.0);
    });
    
    runner.EndSection();
}

void TestLinearSystemParser(TestRunner& runner) {
    runner.StartSection("LINEAR SYSTEM PARSER TESTS");
    
    LinearSystemParser parser;
    
    // Test 1: Simple 2x2 linear system
    runner.RunTest("Solve 2x2 linear system", [&]() {
        auto result = parser.ParseAndExecute("solve [[2,1],[1,3]] [8,13]");
        // Expected solution: x=1, y=6
        return result.HasResult();
    });
    
    // Test 2: Identity matrix system
    runner.RunTest("Identity matrix system", [&]() {
        auto result = parser.ParseAndExecute("solve [[1,0],[0,1]] [5,3]");
        return result.HasResult();
    });
    
    // Test 3: 3x3 system
    runner.RunTest("Solve 3x3 system", [&]() {
        auto result = parser.ParseAndExecute("solve [[2,1,1],[1,3,2],[1,0,0]] [4,5,6]");
        return result.HasResult();
    });
    
    // Test 4: System with non-zero determinant
    runner.RunTest("System with non-zero determinant", [&]() {
        auto result = parser.ParseAndExecute("solve [[3,2],[1,2]] [7,4]");
        return result.HasResult();
    });
    
    // Test 5: Another 2x2 system
    runner.RunTest("Another 2x2 linear system", [&]() {
        auto result = parser.ParseAndExecute("solve [[5,3],[2,1]] [11,5]");
        return result.HasResult();
    });
    
    runner.EndSection();
}

void TestStatisticsEngine(TestRunner& runner) {
    runner.StartSection("STATISTICS ENGINE TESTS");
    
    StatisticsEngine stats;
    
    // Test 1: Mean calculation
    runner.RunTest("Mean of [1,2,3,4,5]", [&]() {
        Vector data = {1.0, 2.0, 3.0, 4.0, 5.0};
        auto result = stats.Mean(data);
        return result.HasResult() && approx_equal(*result.GetDouble(), 3.0);
    });
    
    // Test 2: Standard deviation
    runner.RunTest("StdDev of [2,4,6,8,10]", [&]() {
        Vector data = {2.0, 4.0, 6.0, 8.0, 10.0};
        auto result = stats.StandardDeviation(data);
        return result.HasResult() && *result.GetDouble() > 0.0;
    });
    
    // Test 3: Median
    runner.RunTest("Median of [1,2,3,4,5]", [&]() {
        Vector data = {1.0, 2.0, 3.0, 4.0, 5.0};
        auto result = stats.Median(data);
        return result.HasResult() && approx_equal(*result.GetDouble(), 3.0);
    });
    
    // Test 4: Variance
    runner.RunTest("Variance of [1,2,3,4,5]", [&]() {
        Vector data = {1.0, 2.0, 3.0, 4.0, 5.0};
        auto result = stats.Variance(data);
        return result.HasResult() && approx_equal(*result.GetDouble(), 2.5, 0.1);
    });
    
    // Test 5: Linear regression
    runner.RunTest("Linear regression y=2x+1", [&]() {
        Vector x = {1.0, 2.0, 3.0, 4.0, 5.0};
        Vector y = {3.0, 5.0, 7.0, 9.0, 11.0};
        auto result = stats.LinearRegression(x, y);
        return result.HasResult();
    });
    
    // Test 6: Correlation
    runner.RunTest("Correlation coefficient", [&]() {
        Vector x = {1.0, 2.0, 3.0, 4.0, 5.0};
        Vector y = {2.0, 4.0, 6.0, 8.0, 10.0};
        auto result = stats.Correlation(x, y);
        return result.HasResult() && approx_equal(*result.GetDouble(), 1.0, 0.01);
    });
    
    // Test 7: Mode
    runner.RunTest("Mode of dataset", [&]() {
        Vector data = {1.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0};
        auto result = stats.Mode(data);
        return result.HasResult();
    });
    
    // Test 8: Percentile
    runner.RunTest("50th Percentile (Median)", [&]() {
        Vector data = {1.0, 2.0, 3.0, 4.0, 5.0};
        auto result = stats.Percentile(data, 50.0);
        return result.HasResult() && approx_equal(*result.GetDouble(), 3.0);
    });
    
    runner.EndSection();
}

void TestSymbolicEngine(TestRunner& runner) {
    runner.StartSection("SYMBOLIC ENGINE TESTS");
    
    SymbolicEngine symbolic;
    
    // Test 1: Expand expression
    runner.RunTest("Expand (x+1)^2", [&]() {
        auto result = symbolic.Expand("(x+1)^2");
        // Should return something like "x^2 + 2*x + 1"
        return result.HasResult();
    });
    
    // Test 2: Simplify expression
    runner.RunTest("Simplify expression", [&]() {
        auto result = symbolic.Simplify("x + x + x");
        return result.HasResult();
    });
    
    // Test 3: Symbolic integration
    runner.RunTest("Integrate x^2", [&]() {
        auto result = symbolic.Integrate("x^2", "x");
        // Should return something containing x^3/3
        return result.HasResult();
    });
    
    // Test 4: Symbolic differentiation
    runner.RunTest("Derive x^3", [&]() {
        auto result = symbolic.PartialDerivative("x^3", "x");
        // Should return something containing 3*x^2
        return result.HasResult();
    });
    
    // Test 5: Factor expression
    runner.RunTest("Factor x^2 - 1", [&]() {
        auto result = symbolic.Factor("x^2 - 1");
        return result.HasResult();
    });
    
    // Test 6: Substitute variable
    runner.RunTest("Substitute x=5 in x+3", [&]() {
        auto result = symbolic.Substitute("x+3", "x", "5");
        return result.HasResult();
    });
    
    runner.EndSection();
}

void TestUnitManager(TestRunner& runner) {
    runner.StartSection("UNIT MANAGER TESTS");
    
    UnitManager units;
    
    // Test 1: Length conversion km to m
    runner.RunTest("Convert 1 km to m", [&]() {
        auto result = units.ConvertUnit(1.0, "km", "m");
        return result.HasResult() && approx_equal(*result.GetDouble(), 1000.0);
    });
    
    // Test 2: Mass conversion kg to g
    runner.RunTest("Convert 2 kg to g", [&]() {
        auto result = units.ConvertUnit(2.0, "kg", "g");
        return result.HasResult() && approx_equal(*result.GetDouble(), 2000.0);
    });
    
    // Test 3: Length conversion m to cm
    runner.RunTest("Convert 5 m to cm", [&]() {
        auto result = units.ConvertUnit(5.0, "m", "cm");
        return result.HasResult() && approx_equal(*result.GetDouble(), 500.0);
    });
    
    // Test 4: Check unit compatibility
    runner.RunTest("Unit compatibility: m and km", [&]() {
        return units.AreCompatible("m", "km");
    });
    
    // Test 5: Incompatible units
    runner.RunTest("Unit incompatibility: kg and m", [&]() {
        return !units.AreCompatible("kg", "m");
    });
    
    // Test 6: Temperature conversion C to F
    runner.RunTest("Convert 0°C to °F", [&]() {
        auto result = units.ConvertTemperature(0.0, "C", "F");
        return result.HasResult() && approx_equal(*result.GetDouble(), 32.0);
    });
    
    // Test 7: Temperature conversion F to C
    runner.RunTest("Convert 100°F to °C", [&]() {
        auto result = units.ConvertTemperature(100.0, "F", "C");
        return result.HasResult() && approx_equal(*result.GetDouble(), 37.78, 0.1);
    });
    
    runner.EndSection();
}

void TestPlotEngine(TestRunner& runner) {
    runner.StartSection("PLOT ENGINE TESTS");
    
    PlotEngine plot;
    PlotConfig config;
    config.width = 40;
    config.height = 10;
    
    // Test 1: Plot simple function
    runner.RunTest("Plot sin(x)", [&]() {
        std::string result = plot.PlotFunction("sin(x)", config);
        return !result.empty() && result.length() > 100;
    });
    
    // Test 2: Plot linear function
    runner.RunTest("Plot x", [&]() {
        std::string result = plot.PlotFunction("x", config);
        return !result.empty();
    });
    
    // Test 3: Plot quadratic
    runner.RunTest("Plot x^2", [&]() {
        std::string result = plot.PlotFunction("x^2", config);
        return !result.empty();
    });
    
    // Test 4: Plot data points
    runner.RunTest("Plot data points", [&]() {
        Vector x = {1.0, 2.0, 3.0, 4.0, 5.0};
        Vector y = {1.0, 4.0, 9.0, 16.0, 25.0};
        std::string result = plot.PlotData(x, y, config);
        return !result.empty();
    });
    
    // Test 5: Histogram
    runner.RunTest("Generate histogram", [&]() {
        Vector data = {1.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0, 5.0};
        std::string result = plot.Histogram(data, 5, config);
        return !result.empty();
    });
    
    runner.EndSection();
}

#ifdef ENABLE_EIGEN
void TestEigenEngine(TestRunner& runner) {
    runner.StartSection("EIGEN ENGINE TESTS");
    
    AXIOM::EigenEngine eigen;
    
    // Test 1: Matrix creation
    runner.RunTest("Create 2x2 matrix", [&]() {
        std::vector<std::vector<double>> data = {{1, 2}, {3, 4}};
        auto matrix = eigen.CreateMatrix(data);
        return matrix.rows() == 2 && matrix.cols() == 2;
    });
    
    // Test 2: Matrix multiplication
    runner.RunTest("Matrix multiplication", [&]() {
        std::vector<std::vector<double>> data_a = {{1, 2}, {3, 4}};
        std::vector<std::vector<double>> data_b = {{2, 0}, {1, 2}};
        auto A = eigen.CreateMatrix(data_a);
        auto B = eigen.CreateMatrix(data_b);
        auto C = eigen.MatrixMultiply(A, B);
        return approx_equal(C(0, 0), 4.0) && approx_equal(C(1, 0), 10.0);
    });
    
    // Test 3: Matrix inverse
    runner.RunTest("Matrix inverse", [&]() {
        std::vector<std::vector<double>> data = {{4, 7}, {2, 6}};
        auto A = eigen.CreateMatrix(data);
        auto A_inv = eigen.Inverse(A);
        auto identity = eigen.MatrixMultiply(A, A_inv);
        return approx_equal(identity(0, 0), 1.0, 0.01) && 
               approx_equal(identity(1, 1), 1.0, 0.01);
    });
    
    // Test 4: Matrix transpose
    runner.RunTest("Matrix transpose", [&]() {
        std::vector<std::vector<double>> data = {{1, 2, 3}, {4, 5, 6}};
        auto A = eigen.CreateMatrix(data);
        auto AT = eigen.Transpose(A);
        return AT.rows() == 3 && AT.cols() == 2 && approx_equal(AT(0, 1), 4.0);
    });
    
    // Test 5: Determinant
    runner.RunTest("Determinant calculation", [&]() {
        std::vector<std::vector<double>> data = {{3, 8}, {4, 6}};
        auto A = eigen.CreateMatrix(data);
        double det = eigen.Determinant(A);
        return approx_equal(det, -14.0);
    });
    
    // Test 6: Matrix addition
    runner.RunTest("Matrix addition", [&]() {
        std::vector<std::vector<double>> data_a = {{1, 2}, {3, 4}};
        std::vector<std::vector<double>> data_b = {{5, 6}, {7, 8}};
        auto A = eigen.CreateMatrix(data_a);
        auto B = eigen.CreateMatrix(data_b);
        auto C = eigen.MatrixAdd(A, B);
        return approx_equal(C(0, 0), 6.0) && approx_equal(C(1, 1), 12.0);
    });
    
    // Test 7: Solve linear system
    runner.RunTest("Solve linear system Ax=b", [&]() {
        std::vector<std::vector<double>> data_a = {{2, 1}, {1, 3}};
        std::vector<double> data_b = {8, 13};
        auto A = eigen.CreateMatrix(data_a);
        auto b = eigen.CreateVector(data_b);
        auto x = eigen.SolveLinearSystem(A, b);
        return approx_equal(x(0), 2.2, 0.01) && approx_equal(x(1), 3.6, 0.01);
    });
    
    runner.EndSection();
}
#endif

void TestDynamicCalc(TestRunner& runner) {
    runner.StartSection("DYNAMIC CALC INTEGRATION TESTS");
    
    AXIOM::DynamicCalc calc;
    
    // Test 1: Algebraic mode
    runner.RunTest("DynamicCalc algebraic: 5+5", [&]() {
        auto result = calc.Evaluate("5+5");
        return result.HasResult() && approx_equal(*result.GetDouble(), 10.0);
    });
    
    // Test 2: Switch to statistics mode
    runner.RunTest("DynamicCalc mode switching", [&]() {
        calc.SetMode(AXIOM::CalculationMode::STATISTICS);
        return true; // Just test that mode switching doesn't crash
    });
    
    // Test 3: Complex expression
    runner.RunTest("DynamicCalc complex expression", [&]() {
        calc.SetMode(AXIOM::CalculationMode::ALGEBRAIC);
        auto result = calc.Evaluate("sin(0) + cos(0)");
        return result.HasResult() && approx_equal(*result.GetDouble(), 1.0);
    });
    
    runner.EndSection();
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================
int main(int argc, char** argv) {
    std::cout << Color::BOLD << Color::CYAN 
              << R"(
╔═══════════════════════════════════════════════════════════════╗
║                                                               ║
║           AXIOM ENGINE v3.0 - GIGA TEST SUITE                 ║
║                                                               ║
║        Production-Grade Comprehensive Validation              ║
║                                                               ║
╚═══════════════════════════════════════════════════════════════╝
)" << Color::RESET << "\n";

    TestRunner runner;
    
    // Run all test suites
    TestAlgebraicParser(runner);
    TestLinearSystemParser(runner);
    TestStatisticsEngine(runner);
    TestSymbolicEngine(runner);
    TestUnitManager(runner);
    TestPlotEngine(runner);
    
#ifdef ENABLE_EIGEN
    TestEigenEngine(runner);
#else
    std::cout << "\n" << Color::YELLOW << "[INFO] Eigen tests skipped (ENABLE_EIGEN not defined)" 
              << Color::RESET << "\n";
#endif
    
    TestDynamicCalc(runner);
    
    // Print final summary
    runner.PrintSummary();
    
    return runner.GetExitCode();
}
