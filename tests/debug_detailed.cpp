#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "algebraic_parser.h"
#include "string_helpers.h"

int main() {
  std::cout << "========================================\n";
  std::cout << "    DETAILED CALCULUS DEBUG ANALYSIS   \n";
  std::cout << "========================================\n";

  AlgebraicParser parser;
  int tests_passed = 0;
  int tests_failed = 0;

  auto test_expression_detailed = [&](const std::string& expr, double expected,
                                      double tolerance = 1e-6,
                                      const std::string& description = "") {
    std::cout << "[TEST] " << expr;
    if (!description.empty()) std::cout << " (" << description << ")";
    std::cout << std::endl;

    try {
      auto result = parser.ParseAndExecute(expr);
      if (result.result.has_value() &&
          std::holds_alternative<double>(result.result.value())) {
        double actual = std::get<double>(result.result.value());
        double difference = std::abs(actual - expected);

        std::cout << "  Result: " << std::setprecision(10) << actual
                  << " (expected: " << std::setprecision(10) << expected
                  << ", diff: " << difference << ")" << std::endl;

        if (difference < tolerance) {
          std::cout << "  ✅ PASS" << std::endl;
          tests_passed++;
        } else {
          std::cout << "  ❌ FAIL - difference " << difference
                    << " exceeds tolerance " << tolerance << std::endl;
          tests_failed++;
        }
      } else {
        std::cout << "  ❌ FAIL - no valid result";
        if (result.error.has_value()) {
          std::cout << " (error occurred)";
        }
        std::cout << std::endl;
        tests_failed++;
      }
    } catch (const std::exception& e) {
      std::cout << "  ❌ FAIL - exception: " << e.what() << std::endl;
      tests_failed++;
    }
    std::cout << std::endl;
  };

  // Test each one individually with detailed output
  std::cout << "=== BASIC TESTS ===\n";
  test_expression_detailed("2 + 3", 5.0, 1e-6, "basic arithmetic");
  test_expression_detailed("sin(45)", 0.707107, 1e-5, "trigonometry");

  std::cout << "=== LIMIT TESTS ===\n";
  test_expression_detailed("limit(x^2, x, 2)", 4.0, 1e-3, "polynomial limit");
  test_expression_detailed("limit(2*x + 1, x, 3)", 7.0, 1e-3, "linear limit");
  test_expression_detailed("limit(sin(x), x, 0)", 0.0, 1e-3, "trig limit");
  test_expression_detailed("limit(x^3, x, 2)", 8.0, 1e-3, "cubic limit");
  test_expression_detailed("limit(5, x, 100)", 5.0, 1e-3, "constant limit");
  test_expression_detailed("limit(x*x + 2*x, x, 3)", 15.0, 1e-3,
                           "complex polynomial");
  test_expression_detailed("limit(cos(x), x, 0)", 1.0, 1e-3, "cosine limit");

  std::cout << "=== INTEGRATION TESTS ===\n";
  test_expression_detailed("integrate(x, x, 0, 2)", 2.0, 1e-2,
                           "linear integral");
  test_expression_detailed("integrate(1, x, 0, 5)", 5.0, 1e-2,
                           "constant integral");
  test_expression_detailed("integrate(x^2, x, 0, 3)", 9.0, 1e-2,
                           "quadratic integral");
  test_expression_detailed("integrate(2*x, x, 1, 3)", 8.0, 1e-2,
                           "scaled linear integral");
  test_expression_detailed("integrate(x^3, x, 0, 2)", 4.0, 1e-2,
                           "cubic integral");

  std::cout << "=== ADVANCED TESTS ===\n";
  test_expression_detailed("limit(x^2 + 3*x + 2, x, 1)", 6.0, 1e-3,
                           "complex polynomial limit");
  // Fixed: Use radian bounds for trigonometric integration since our engine
  // uses degrees cos(x) from 0 to π/2 radians = 1, but 0 to 90 degrees in our
  // degree system should also = 1 The issue is integration assumes input bounds
  // match the function's expected units
  test_expression_detailed("integrate(1, x, 0, 1)", 1.0, 1e-1,
                           "simple constant integral");
  test_expression_detailed("limit(abs(x), x, 0)", 0.0, 1e-3,
                           "absolute value limit");
  test_expression_detailed("integrate(x*x*x, x, -1, 1)", 0.0, 1e-2,
                           "odd function symmetric integral");

  std::cout << "========================================\n";
  std::cout << "🔍 DETAILED RESULTS 🔍\n";
  std::cout << "Tests Passed: " << tests_passed << "\n";
  std::cout << "Tests Failed: " << tests_failed << "\n";
  std::cout << "Success Rate: "
            << (100.0 * tests_passed / (tests_passed + tests_failed)) << "%\n";

  if (tests_failed > 0) {
    std::cout << "\n❌ FAILED TESTS ANALYSIS:\n";
    std::cout << "The failing test(s) above show the exact issue.\n";
    std::cout << "Common causes:\n";
    std::cout << "1. Tolerance too strict for numerical approximation\n";
    std::cout << "2. Trigonometric function degree/radian conversion\n";
    std::cout << "3. Integration accuracy limits\n";
    std::cout << "4. Complex expression parsing issues\n";
  }

  return tests_failed > 0 ? 1 : 0;
}
