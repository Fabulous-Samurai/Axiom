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

#include "axiom_export.h"
#include "fixed_vector.h"

namespace Utils {
    
    // Fast string-to-double conversion using std::from_chars (C++17) or std::strtod fallback
    inline std::optional<double> FastParseDouble(std::string_view sv) {
        if (sv.empty() || sv.size() > 125) return std::nullopt;
        
        char buf[128];
        size_t len = 0;
        
        // Handle edge cases
        if (sv.front() == '.') {
            buf[len++] = '0';
        }

        for (char c : sv) {
            buf[len++] = c;
        }
        
        if (sv.back() == '.') {
            buf[len++] = '0';
        }

        double result;
#if defined(__apple_build_version__) || (defined(__GNUC__) && __GNUC__ < 11 && !defined(__clang__))
        // Exception-free fallback for compilers with missing floating-point from_chars
        buf[len] = '\0';
        char* end_ptr;
        errno = 0;
        result = std::strtod(buf, &end_ptr);

        if (end_ptr != buf + len) return std::nullopt;
        if (errno == ERANGE && result != 0.0) return std::nullopt; // Allow underflow (result == 0.0)

        return result;
#else
        auto [ptr, ec] = std::from_chars(buf, buf + len, result);
        // Check if conversion was successful AND we consumed the entire string
        return (ec == std::errc{} && ptr == buf + len) ? std::optional<double>(result) : std::nullopt;
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
