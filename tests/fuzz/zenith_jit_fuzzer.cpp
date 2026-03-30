#include <cstdint>
#include <cstddef>
#include <string>
#include <unordered_map>
#include "algebraic_parser.h"
#include "zenith_jit.h"

using namespace AXIOM;

// Fuzz target for AlgebraicParser and ZenithJIT
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // We need at least some bytes to make a meaningful expression
    if (size == 0) {
        return 0;
    }

    // Convert fuzz data to a string
    std::string expr(reinterpret_cast<const char*>(data), size);

    // Context for compilation
    AlgebraicParser parser;
    ZenithJIT jit;
    
    // We just want to check that parsing malformed ASTs / strings 
    // does not crash the system (Zero-Exception policy + memory safety).
    
    NodePtr root = parser.ParseExpression(expr);
    if (!root) {
        return 0; // Failed to parse, which is expected for random bytes
    }

    // If it parsed successfully, try to compile it
    SymbolTable var_map;
    var_map.push_back({"x", 0.0});
    var_map.push_back({"y", 0.0});
    var_map.push_back({"z", 0.0});
    var_map.push_back({"M", 0.0});

    // Fuzz Scalar Compilation
    JiffedFunc fn = jit.Compile(root, var_map);
    if (fn) {
        // Just execute it with zeroed variables
        double vars[3] = {0.0, 0.0, 0.0};
        fn(vars);
    }
    
    // Fuzz Matrix Compilation
    JiffedMatrixFunc mfn = jit.CompileMatrix(root, var_map);
    if (mfn) {
        double flat_matrix[16] = {0};
        double results[16] = {0};
        mfn(flat_matrix, results);
    }

    return 0;
}

#ifdef _WIN32
int main() {
    // Dummy main for Windows stabilization
    return 0;
}
#endif
