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
        
        // Handle edge cases that std::from_chars might not handle well without allocation
        // Handle leading decimal point (e.g., ".5" -> "0.5")
        // Handle trailing decimal point (e.g., "5." -> "5.0")
        if (sv.front() == '.' || sv.back() == '.') {
            std::string str(sv);
            if (str.front() == '.') {
                str = "0" + str;
            } else if (str.back() == '.') {
                str += "0";
            }
            double result;
#if defined(__apple_build_version__) || (defined(__GNUC__) && __GNUC__ < 11 && !defined(__clang__))
            try {
                size_t pos;
                result = std::stod(str, &pos);
                if (pos != str.size()) return std::nullopt;
                return result;
            } catch (...) {
                return std::nullopt;
            }
#else
            auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
            return (ec == std::errc{} && ptr == str.data() + str.size()) ? std::optional<double>(result) : std::nullopt;
#endif
        }
        
        double result;
#if defined(__apple_build_version__) || (defined(__GNUC__) && __GNUC__ < 11 && !defined(__clang__))
        std::string str(sv);
        try {
            size_t pos;
            result = std::stod(str, &pos);
            if (pos != str.size()) return std::nullopt;
            return result;
        } catch (...) {
            return std::nullopt;
        }
#else
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
        return (ec == std::errc{} && ptr == sv.data() + sv.size()) ? std::optional<double>(result) : std::nullopt;
#endif
    }

    // Helper to trim strings (removes whitespace from both ends)
    AXIOM_EXPORT std::string Trim(std::string_view str);
    
    // Internal helper for non-allocating trim
    std::string_view TrimView(std::string_view str);

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
