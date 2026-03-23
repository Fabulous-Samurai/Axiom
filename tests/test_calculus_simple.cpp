#include <iostream>
#include <cmath>
#include <cassert>

// Simple test for calculus functions without full engine dependencies
#include "algebraic_parser.h"
#include "string_helpers.h"

int main() {
    std::cout << "========================================\n";
    std::cout << "    CALCULUS FUNCTIONS TEST SUITE      \n";
    std::cout << "========================================\n";

    AXIOM::AlgebraicParser parser;
    int tests_passed = 0;
    int tests_failed = 0;

    auto test_expression = [&](const std::string& expr, double expected, double tolerance = 1e-6) {
        std::cout << "[TEST] " << expr << std::endl;
        try {
            auto result = parser.ParseAndExecute(expr);
            if (result.result.has_value() && std::holds_alternative<double>(result.result.value())) {
                double actual = std::get<double>(result.result.value());
                std::cout << "  Result: " << actual << " (expected: " << expected << ")" << std::endl;
                
                if (std::abs(actual - expected) < tolerance) {
                    std::cout << "   PASS" << std::endl;
                    tests_passed++;
                } else {
                    std::cout << "   FAIL - difference: " << std::abs(actual - expected) << std::endl;
                    tests_failed++;
                }
            } else {
                std::cout << "   FAIL - no valid result" << std::endl;
                tests_failed++;
            }
        } catch (const std::exception& e) {
            std::cout << "   FAIL - exception: " << e.what() << std::endl;
            tests_failed++;
        }
        std::cout << std::endl;
    };

    // Test basic expressions first
    std::cout << "=== BASIC EXPRESSION TESTS ===\n";
    test_expression("2 + 3", 5.0);
    // Note: x^2 fails without context, which is expected behavior

    // Test limit function
    std::cout << "=== LIMIT TESTS ===\n";
    test_expression("limit(x^2, x, 2)", 4.0, 1e-3);
    test_expression("limit(2*x + 1, x, 3)", 7.0, 1e-3);
    test_expression("limit(sin(x), x, 0)", 0.0, 1e-3);
    test_expression("limit(x^3, x, 2)", 8.0, 1e-3);
    test_expression("limit(5, x, 100)", 5.0, 1e-3); // constant function
    
    // Advanced limit tests
    test_expression("limit(x*x + 2*x, x, 3)", 15.0, 1e-3); // 9 + 6 = 15
    test_expression("limit(cos(x), x, 0)", 1.0, 1e-3); // cos(0) = 1

    // Test integrate function  
    std::cout << "=== INTEGRATION TESTS ===\n";
    test_expression("integrate(x, x, 0, 2)", 2.0, 1e-2);
    test_expression("integrate(1, x, 0, 5)", 5.0, 1e-2);
    test_expression("integrate(x^2, x, 0, 3)", 9.0, 1e-2);

    // Test error cases
    std::cout << "=== ERROR HANDLING TESTS ===\n";
    std::cout << "[TEST] limit(x) - should fail with wrong argument count\n";
    try {
        auto result = parser.ParseAndExecute("limit(x)");
        if (!result.result.has_value()) {
            std::cout << "   PASS - correctly failed\n";
            tests_passed++;
        } else {
            std::cout << "   FAIL - should have failed\n";
            tests_failed++;
        }
    } catch (...) {
        std::cout << "   PASS - correctly threw exception\n";
        tests_passed++;
    }

    std::cout << "========================================\n";
    std::cout << "Tests Passed: " << tests_passed << "\n";
    std::cout << "Tests Failed: " << tests_failed << "\n";
    
    if (tests_failed == 0) {
        std::cout << " ALL CALCULUS TESTS PASSED!\n";
        return 0;
    } else {
        std::cout << "  SOME TESTS FAILED\n";
        return 1;
    }
}
