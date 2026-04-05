#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "algebraic_parser.h"
#include "zenith_jit.h"

using namespace AXIOM;

void TestBasicArithmetic() {
    std::cout << "[ZenithJIT] Testing Basic Arithmetic..." << std::endl;
    AlgebraicParser parser;
    ZenithJIT jit;

    std::string_view expr = "2 + 3 * 4 - 8 / 2"; // 2 + 12 - 4 = 10
    NodePtr root = parser.ParseExpression(expr);
    assert(root != nullptr);

    SymbolTable var_map;
    JiffedFunc fn = jit.Compile(root, var_map);
    assert(fn != nullptr);

    double result = fn(nullptr);
    std::cout << "  Result of '" << expr << "' is " << result << " (Expected 10.0)" << std::endl;
    assert(std::abs(result - 10.0) < 1e-9);
    std::cout << "  Arithmetic Test Passed!" << std::endl;
}

void TestVariables() {
    std::cout << "[ZenithJIT] Testing Variables..." << std::endl;
    AlgebraicParser parser;
    ZenithJIT jit;

    std::string_view expr = "x * x + y"; // x=3, y=5 => 3*3+5 = 14
    NodePtr root = parser.ParseExpression(expr);
    assert(root != nullptr);

    SymbolTable var_map;
    var_map.push_back({"x", 0.0});
    var_map.push_back({"y", 0.0});

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
    AlgebraicParser parser;
    ZenithJIT jit;

    std::string_view expr = "(a + b) * (a - b) / (a * a + b * b)"; // a=4, b=2 => (6 * 2) / (16 + 4) = 12 / 20 = 0.6
    NodePtr root = parser.ParseExpression(expr);
    assert(root != nullptr);

    SymbolTable var_map;
    var_map.push_back({"a", 0.0});
    var_map.push_back({"b", 0.0});

    JiffedFunc fn = jit.Compile(root, var_map);
    assert(fn != nullptr);

    double vars[] = {4.0, 2.0};
    double result = fn(vars);
    std::cout << "  Result is " << result << " (Expected 0.6)" << std::endl;
    assert(std::abs(result - 0.6) < 1e-9);
    std::cout << "  Nested Complexity Test Passed!" << std::endl;
}

void TestMatrixSIMDMath() {
    std::cout << "[ZenithJIT] Testing Matrix SIMD Math (sin)..." << std::endl;
    AlgebraicParser parser;
    ZenithJIT jit;

    Matrix m;
    Vector r1; r1.push_back(0.0); r1.push_back(30.0); r1.push_back(45.0); r1.push_back(60.0);
    Vector r2; r2.push_back(90.0); r2.push_back(120.0); r2.push_back(135.0); r2.push_back(150.0);
    Vector r3; r3.push_back(180.0); r3.push_back(210.0); r3.push_back(225.0); r3.push_back(240.0);
    Vector r4; r4.push_back(270.0); r4.push_back(300.0); r4.push_back(315.0); r4.push_back(330.0);
    m.push_back(std::move(r1));
    m.push_back(std::move(r2));
    m.push_back(std::move(r3));
    m.push_back(std::move(r4));

    std::string_view expr = "sin(M)";
    std::cout << "  Parsing expression: " << expr << std::endl;
    NodePtr root = parser.ParseExpression(expr);
    assert(root != nullptr);

    std::cout << "  Compiling matrix expression..." << std::endl;
    SymbolTable var_map;
    var_map.push_back({"M", 0.0});

    JiffedMatrixFunc fn = jit.CompileMatrix(root, var_map);
    if (!fn) {
        std::cerr << "  Compilation failed!" << std::endl;
        assert(false);
    }

    std::cout << "  Preparing data and calling JIT function..." << std::endl;
    double flat_matrix[16];
    for(int i=0; i<4; ++i) {
        for(int j=0; j<4; ++j) flat_matrix[i*4 + j] = m[i][j];
    }

    double results[16] = {0};
    fn(flat_matrix, results);

    std::cout << "  Verifying results..." << std::endl;
    const double D2R = 3.14159265358979323846 / 180.0;
    for(int i=0; i<4; ++i) {
        for(int j=0; j<4; ++j) {
            double expected = std::sin(m[i][j] * D2R);
            double actual = results[i*4 + j];
            if (std::abs(expected - actual) > 1e-7) {
                std::cerr << "    Mismatch at [" << i << "][" << j << "]: "
                          << "Expected " << expected << ", got " << actual << std::endl;
            }
        }
    }
    std::cout << "  Matrix SIMD Math Test Finished!" << std::endl;
}

int main() {
    TestBasicArithmetic();
    TestVariables();
    TestNestedComplexity();
    TestMatrixSIMDMath();
    std::cout << "[ZenithJIT] ALL TESTS PASSED!" << std::endl;
    return 0;
}
