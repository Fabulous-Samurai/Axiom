#include <gtest/gtest.h>
#include "algebraic_parser.h"
#include "zenith_jit.h"
#include "symbolic_engine.h"
#include "dynamic_calc.h"
#include <iostream>
#include <random>
#include <cmath>

class DifferentialTest : public ::testing::Test {
protected:
    AXIOM::AlgebraicParser parser;
    AXIOM::ZenithJIT jit;
    SymbolicEngine symbolic;
};

TEST_F(DifferentialTest, RandomExpressionConsistency) {
    // A list of operations to build random expressions
    std::vector<std::string> ops = {"+", "-", "*", "/"};
    std::vector<std::string> funcs = {"sin", "cos", "exp", "abs"};
    
    std::random_device rd; // NOSONAR: This is for testing consistency and rd is generally safer
    std::mt19937_64 gen(rd()); 
    std::uniform_real_distribution<> dis(1.0, 10.0);
    std::uniform_int_distribution<> op_dis(0, ops.size() - 1);
    std::uniform_int_distribution<> func_dis(0, funcs.size() - 1);

    for (int i = 0; i < 50; ++i) {
        double v1 = dis(gen);
        double v2 = dis(gen);
        
        // Construct a simple expression: func(v1) op v2
        std::string expr = funcs[func_dis(gen)] + "(" + std::to_string(v1) + ") " + ops[op_dis(gen)] + " " + std::to_string(v2);
        
        // 1. Interpreted Result
        auto res_interp = parser.ParseAndExecute(expr);
        if (!res_interp.HasResult()) continue;
        double val_interp = res_interp.GetDouble().value_or(0.0);

        // 2. JIT Result
        auto root = parser.ParseExpression(expr);
        if (root) {
            auto jit_fn = jit.Compile(root, {});
            if (jit_fn) {
                double val_jit = jit_fn(nullptr);
                EXPECT_NEAR(val_interp, val_jit, 1e-7) << "JIT mismatch for: " << expr;
            }
        }

        // 3. Symbolic Simplify Result (Evaluates constant expressions)
        auto res_sym = symbolic.Simplify(expr);
        if (res_sym.result.has_value()) {
            std::string sym_str = std::get<std::string>(*res_sym.result);
            try {
                double val_sym = std::stod(sym_str);
                EXPECT_NEAR(val_interp, val_sym, 1e-7) << "Symbolic mismatch for: " << expr;
            } catch (...) {
                // If it couldn't simplify to a number, skip
            }
        }
    }
}
