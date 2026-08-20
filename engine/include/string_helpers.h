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
        
        double result;

#if defined(__apple_build_version__) || (defined(__GNUC__) && __GNUC__ < 11 && !defined(__clang__))
        std::string str(sv);
        // Handle leading decimal point (e.g., ".5" -> "0.5")
        if (str.front() == '.') {
            str = "0" + str;
        }
        // Handle trailing decimal point (e.g., "5." -> "5.0")
        else if (str.back() == '.') {
            str += "0";
        }
        // Fallback for compilers with missing floating-point from_chars
        try {
            size_t pos;
            result = std::stod(str, &pos);
            if (pos != str.size()) return std::nullopt;
            return result;
        } catch (...) {
            return std::nullopt;
        }
#else
        // Fast path: Try parsing without any allocations
        std::string_view parse_sv = sv;
        if (parse_sv.front() == '+') {
            parse_sv.remove_prefix(1);
        }
        auto [ptr, ec] = std::from_chars(parse_sv.data(), parse_sv.data() + parse_sv.size(), result);
        if (ec == std::errc{} && ptr == parse_sv.data() + parse_sv.size()) {
            return result;
        }

        // Slow path: Handle edge cases (e.g., ".5", "5.") by allocating a string
        std::string str(sv);
        // Handle leading decimal point (e.g., ".5" -> "0.5")
        if (str.front() == '.') {
            str = "0" + str;
        } else if (str.front() == '+' && str.size() > 1 && str[1] == '.') {
            str = "+0." + str.substr(2);
        }
        // Handle trailing decimal point (e.g., "5." -> "5.0")
        else if (str.back() == '.') {
            str += "0";
        }

        std::string_view fallback_sv = str;
        if (fallback_sv.front() == '+') {
            fallback_sv.remove_prefix(1);
        }

        auto [ptr2, ec2] = std::from_chars(fallback_sv.data(), fallback_sv.data() + fallback_sv.size(), result);
        return (ec2 == std::errc{} && ptr2 == fallback_sv.data() + fallback_sv.size()) ? std::optional<double>(result) : std::nullopt;
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
