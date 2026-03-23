#include <iostream>
#include <cmath>

#include "algebraic_parser.h"
#include "string_helpers.h"

using AXIOM::AlgebraicParser;

int main() {
    std::cout << "DEBUG: Testing limit function step by step\n";
    
    AlgebraicParser parser;
    
    // Test 1: Simple expression parsing
    std::cout << "\n=== Testing basic parsing ===\n";
    auto result1 = parser.ParseAndExecute("x^2");
    std::cout << "x^2 result has_value: " << result1.result.has_value() << std::endl;
    
    // Test 2: Function with parentheses  
    std::cout << "\n=== Testing function syntax ===\n";
    auto result2 = parser.ParseAndExecute("sin(45)");
    std::cout << "sin(45) result: " << (result2.result.has_value() ? "SUCCESS" : "FAILED") << std::endl;
    if (result2.result.has_value()) {
        std::cout << "sin(45) = " << std::get<double>(result2.result.value()) << std::endl;
    }
    
    // Test 3: Multi-argument parsing
    std::cout << "\n=== Testing multi-arg function recognition ===\n";
    auto result3 = parser.ParseAndExecute("limit(2, x, 1)");
    std::cout << "limit(2, x, 1) result: " << (result3.result.has_value() ? "SUCCESS" : "FAILED") << std::endl;
    if (result3.result.has_value()) {
        std::cout << "limit(2, x, 1) = " << std::get<double>(result3.result.value()) << std::endl;
    } else if (result3.error.has_value()) {
        std::cout << "Error occurred in limit function" << std::endl;
    }
    
    // Test 4: Simple polynomial limit
    std::cout << "\n=== Testing simple polynomial limit ===\n";
    auto result4 = parser.ParseAndExecute("limit(x, x, 5)");
    std::cout << "limit(x, x, 5) result: " << (result4.result.has_value() ? "SUCCESS" : "FAILED") << std::endl;
    if (result4.result.has_value()) {
        std::cout << "limit(x, x, 5) = " << std::get<double>(result4.result.value()) << std::endl;
    }
    
    // Test 5: Check if limit function is being parsed as MultiArgFunctionNode
    std::cout << "\n=== Testing function recognition ===\n";
    
    // Let's test if the issue is in parsing or evaluation
    try {
        std::cout << "Attempting to parse: limit(3*x, x, 2)\n";
        auto result5 = parser.ParseAndExecute("limit(3*x, x, 2)");
        
        if (result5.result.has_value()) {
            std::cout << " SUCCESS: limit(3*x, x, 2) = " << std::get<double>(result5.result.value()) << std::endl;
        } else if (result5.error.has_value()) {
            std::cout << " Error in limit evaluation\n";
        } else {
            std::cout << " Unknown parsing/evaluation issue\n";
        }
    } catch (const std::exception& e) {
        std::cout << " Exception: " << e.what() << std::endl;
    }
    
    return 0;
}
