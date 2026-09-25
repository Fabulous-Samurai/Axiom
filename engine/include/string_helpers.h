// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <charconv>
#include <optional>
#include <string_view>
#include <cstdlib>
#include <cerrno>

#include "axiom_export.h"
#include "fixed_vector.h"

namespace Utils {
    
    // Fast string-to-double conversion using std::from_chars (C++17)
    // ⚡ Bolt: Removed std::string allocations to adhere to Zenith Pillar 1 (Zero-Allocation).
    // Using std::from_chars natively handles floats. For compiler fallbacks, we use a
    // stack-based buffer with std::strtod (and a heap fallback for >64 chars to be safe),
    // removing the costly throw/catch blocks of std::stod to comply with Pillar 5 (Zero-Exception).
    inline std::optional<double> FastParseDouble(std::string_view sv) {
        if (sv.empty()) return std::nullopt;
        
        double result;
#if defined(__apple_build_version__) || (defined(__GNUC__) && __GNUC__ < 11 && !defined(__clang__))
        // Fallback for compilers with missing floating-point from_chars
        constexpr size_t kStackBufSize = 64;
        char stack_buf[kStackBufSize];
        const char* str_ptr = sv.data();
        std::string heap_buf;

        if (sv.size() < kStackBufSize) {
            std::copy(sv.begin(), sv.end(), stack_buf);
            stack_buf[sv.size()] = '\0';
            str_ptr = stack_buf;
        } else {
            heap_buf = std::string(sv);
            str_ptr = heap_buf.c_str();
        }

        char* end = nullptr;
        errno = 0;
        result = std::strtod(str_ptr, &end);

        if (errno == ERANGE && result != 0.0) return std::nullopt;
        if (end != str_ptr + sv.size()) return std::nullopt;

        return result;
#else
        // In C++17 std::from_chars natively supports strings without leading zeros (e.g. ".5")
        // No need to copy to std::string and pad zeros!
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
        // Check if conversion was successful AND we consumed the entire string
        return (ec == std::errc{} && ptr == sv.data() + sv.size()) ? std::optional<double>(result) : std::nullopt;
#endif
    }

    // Helper to trim strings (removes whitespace from both ends)
    AXIOM_EXPORT std::string Trim(std::string_view str);
    
    // Helper to split string by delimiter
    AXIOM_EXPORT AXIOM::FixedVector<std::string, 256> Split(std::string_view s, char delimiter);

    // Modern C++ Way: Exception-free number check with fast parsing
    inline bool IsNumber(std::string_view str) {
        if (str.empty()) return false;
        return FastParseDouble(str).has_value();
    }
    
    // Helper for ReplaceAns logic (Moved from main.cpp)
    inline std::string ReplaceAns(std::string input, double last_val) {
        const std::string search = "Ans";
        size_t pos = 0;
        if (input.find(search) == std::string::npos) return input;

        std::stringstream ss;
        ss.precision(15);
        ss << last_val;
        std::string replace = ss.str();

        while ((pos = input.find(search, pos)) != std::string::npos) {
            input.replace(pos, search.length(), replace);
            pos += replace.length();
        }
        return input;
    }

    // String utilities
    AXIOM_EXPORT std::string ReplaceAll(std::string_view str, std::string_view from, std::string_view to);
}
