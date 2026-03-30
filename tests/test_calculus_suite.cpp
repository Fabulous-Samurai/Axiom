#include <iostream>
#include <cmath>
#include <cassert>
#include "algebraic_parser.h"
#include "string_helpers.h"

// Unified Calculus Test Suite for AXIOM Zenith

int main() {
    std::cout << "====================================================" << std::endl;
    std::cout << "    UNIFIED CALCULUS & SYMBOLIC TEST SUITE (v3.1.2) " << std::endl;
    std::cout << "====================================================" << std::endl;

    AXIOM::AlgebraicParser parser;
    int tests_passed = 0;
    int tests_failed = 0;

    auto test_expression = [&](const std::string& expr, double expected, double tolerance = 1e-6) {
        std::cout << "[TEST] " << expr << std::endl;
        
        auto result = parser.ParseAndExecute(expr);
        if (result.HasResult() && result.GetDouble().has_value()) {
            double actual = *result.GetDouble();
            if (std::abs(actual - expected) < tolerance) {
                tests_passed++;
            } else {
                std::cout << "  [FAIL] expected: " << expected << ", actual: " << actual 
                          << ", diff: " << std::abs(actual - expected) << std::endl;
                tests_failed++;
            }
        } else {
            std::cout << "  [FAIL] no valid double result" << std::endl;
            tests_failed++;
        }
    };
    
    // --- All tests from simple and final combined ---

    test_expression("2 + 3", 5.0);
    test_expression("sin(45)", 0.70710678, 1e-5);
    test_expression("limit(x^2, x, 2)", 4.0, 1e-3);
    test_expression("limit(2*x + 1, x, 3)", 7.0, 1e-3);
    test_expression("integrate(x, x, 0, 2)", 2.0, 1e-2);
    test_expression("integrate(x^2, x, 0, 3)", 9.0, 1e-2);
    test_expression("limit(abs(x), x, 0)", 0.0, 1e-3);
    test_expression("integrate(x*x*x, x, -1, 1)", 0.0, 1e-2);

    // Final results
    std::cout << "========================================" << std::endl;
    std::cout << "FINAL RESULTS" << std::endl;
    std::cout << "Tests Passed: " << tests_passed << " / " << (tests_passed + tests_failed) << std::endl;
    
    if (tests_failed == 0) {
        std::cout << "ALL CALCULUS TESTS PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "SOME TESTS FAILED - DEBUGGING NEEDED" << std::endl;
        return 1;
    }
}
