#include <stdint.h>
#include <stddef.h>
#include <string>
#include "algebraic_parser.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size == 0) return 0;

    std::string input(reinterpret_cast<const char*>(Data), Size);
    AXIOM::AlgebraicParser parser;

    // Attempt to parse and execute
    try {
        auto res = parser.ParseAndExecute(input);
        (void)res;
    } catch (...) {
        // We catch all exceptions; libFuzzer is looking for crashes (SIGSEGV, etc.)
    }

    return 0;
}
