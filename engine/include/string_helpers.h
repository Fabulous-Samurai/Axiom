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
    
    // ⚡ BOLT OPTIMIZATION:
    // What: Replaced std::string allocation in FastParseDouble with a stack-based buffer and fast path.
    // Why: Avoids heap allocation (O(1) overhead) on every double parse, which is highly frequent in parsers.
    // Impact: Significantly reduces latency and satisfies the Zero-Allocation Pillar.
    inline std::optional<double> FastParseDouble(std::string_view sv) {
        if (sv.empty()) return std::nullopt;
        
        double result;

#if defined(__apple_build_version__) || (defined(__GNUC__) && __GNUC__ < 11 && !defined(__clang__))
        // Fallback for compilers without from_chars support for double
        char buf[64];
        size_t len = sv.size();
        if (len > 60) return std::nullopt;

        size_t i = 0;
        if (sv.front() == '.') buf[i++] = '0';
        for (char c : sv) buf[i++] = c;
        if (sv.back() == '.') buf[i++] = '0';
        buf[i] = '\0';

        try {
            size_t pos;
            result = std::stod(buf, &pos);
            if (pos != i) return std::nullopt;
            return result;
        } catch (...) {
            return std::nullopt;
        }
#else
        // Fast path: Most normal numbers don't start or end with a bare decimal.
        if (sv.front() != '.' && sv.back() != '.') {
            auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
            if (ec == std::errc{} && ptr == sv.data() + sv.size()) {
                return result;
            }
        }

        // Slow path: Handle ".5", "5." and un-null-terminated string edge cases using stack buffer.
        char buf[64];
        size_t len = sv.size();
        if (len > 60) return std::nullopt; // Too long for reasonable double representation

        size_t i = 0;
        if (sv.front() == '.') buf[i++] = '0';
        for (char c : sv) buf[i++] = c;
        if (sv.back() == '.') buf[i++] = '0';

        auto [ptr2, ec2] = std::from_chars(buf, buf + i, result);
        return (ec2 == std::errc{} && ptr2 == buf + i) ? std::optional<double>(result) : std::nullopt;
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
