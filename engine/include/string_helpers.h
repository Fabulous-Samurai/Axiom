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
    
    // Fast string-to-double conversion using std::from_chars (C++17)
    inline std::optional<double> FastParseDouble(std::string_view sv) {
        if (sv.empty()) return std::nullopt;
        
        // ⚡ Bolt: Fast string-to-double parsing avoiding std::string allocation
        // C++17 std::from_chars natively supports `.5` and `5.` formats,
        // eliminating the need for padding and string conversions.
        const char* start = sv.data();
        size_t len = sv.size();
        
        if (len > 0 && start[0] == '+') {
            start++;
            len--;
        }
        
        if (len == 0) return std::nullopt;

        double result;
#if defined(__apple_build_version__) || (defined(__GNUC__) && __GNUC__ < 11 && !defined(__clang__))
        // Fallback for older compilers without float from_chars support.
        // We avoid try-catch/std::stod overhead (Zenith Pillar 5).
        char buffer[64];
        if (len >= sizeof(buffer)) {
            // Rare edge case: extremely long number string
            std::string str(start, len);
            char* end;
            result = std::strtod(str.c_str(), &end);
            return (end == str.c_str() + len) ? std::optional<double>(result) : std::nullopt;
        }

        for(size_t i = 0; i < len; ++i) buffer[i] = start[i];
        buffer[len] = '\0';
        char* end;
        result = std::strtod(buffer, &end);
        if (end != buffer + len) return std::nullopt;
        return result;
#else
        auto [ptr, ec] = std::from_chars(start, start + len, result);
        return (ec == std::errc{} && ptr == start + len) ? std::optional<double>(result) : std::nullopt;
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
