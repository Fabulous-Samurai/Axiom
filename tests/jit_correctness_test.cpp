#include <iostream>
#include <cassert>
#include <cmath>
#include <unordered_map>
#include <string>
#include "algebraic_parser.h"
#include "zenith_jit.h"

using namespace AXIOM;

void TestBasicArithmetic() {
    std::cout << "[ZenithJIT] Testing Basic Arithmetic..." << std::endl;
    AXIOM::AlgebraicParser parser;
    ZenithJIT jit;

    std::string expr = "2 + 3 * 4 - 8 / 2"; // 2 + 12 - 4 = 10
    NodePtr root = parser.ParseExpression(expr);
    assert(root != nullptr);

    std::unordered_map<std::string, int> var_map;
    JiffedFunc fn = jit.Compile(root, var_map);
    assert(fn != nullptr);

    double result = fn(nullptr);
    std::cout << "  Result of '" << expr << "' is " << result << " (Expected 10.0)" << std::endl;
    assert(std::abs(result - 10.0) < 1e-9);
    std::cout << "  Arithmetic Test Passed!" << std::endl;
}

void TestVariables() {
    std::cout << "[ZenithJIT] Testing Variables..." << std::endl;
    AXIOM::AlgebraicParser parser;
    ZenithJIT jit;

    std::string expr = "x * x + y"; // x=3, y=5 => 3*3+5 = 14
    NodePtr root = parser.ParseExpression(expr);
    assert(root != nullptr);

    std::unordered_map<std::string, int> var_map = {{"x", 0}, {"y", 1}};
    JiffedFunc fn = jit.Compile(root, var_map);
    assert(fn != nullptr);

    double vars[] = {3.0, 5.0};
    double result = fn(vars);
    std::cout << "  Result of '" << expr << "' with x=3, y=5 is " << result << " (Expected 14.0)" << std::endl;
    assert(std::abs(result - 14.0) < 1e-9);
    std::cout << "  Variables Test Passed!" << std::endl;
}

void TestNestedComplexity() {
    std::cout << "[ZenithJIT] Testing Nested Complexity..." << std::endl;
    AXIOM::AlgebraicParser parser;
    ZenithJIT jit;

    std::string expr = "(a + b) * (a - b) / (a * a + b * b)"; // a=4, b=2 => (6 * 2) / (16 + 4) = 12 / 20 = 0.6
    NodePtr root = parser.ParseExpression(expr);
    assert(root != nullptr);

    std::unordered_map<std::string, int> var_map = {{"a", 0}, {"b", 1}};
    JiffedFunc fn = jit.Compile(root, var_map);
    assert(fn != nullptr);

    double vars[] = {4.0, 2.0};
    double result = fn(vars);
    std::cout << "  Result is " << result << " (Expected 0.6)" << std::endl;
    assert(std::abs(result - 0.6) < 1e-9);
    std::cout << "  Nested Complexity Test Passed!" << std::endl;
}

int main() {
    try {
        TestBasicArithmetic();
        TestVariables();
        TestNestedComplexity();
        std::cout << "[ZenithJIT] ALL TESTS PASSED!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ZenithJIT] TEST FAILED: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

