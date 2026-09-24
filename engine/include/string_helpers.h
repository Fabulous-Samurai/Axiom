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
        // ⚡ Bolt: Zero-allocation fallback for older compilers using stack buffer for typical sizes
        // and avoiding forbidden exceptions (Zenith Pillar 5).
        constexpr size_t MAX_STACK = 64;
        char stack_buf[MAX_STACK];
        char* buf = stack_buf;
        
        if (sv.size() >= MAX_STACK) {
            buf = new char[sv.size() + 1];
        }
        
        std::copy(sv.begin(), sv.end(), buf);
        buf[sv.size()] = '\0';

        char* end;
        errno = 0;
        result = std::strtod(buf, &end);

        // Allow underflow to 0.0 to match std::stod behavior, reject other range errors
        bool success = (end == buf + sv.size()) && !(errno == ERANGE && result != 0.0);

        if (buf != stack_buf) {
            delete[] buf;
        }

        return success ? std::optional<double>(result) : std::nullopt;
#else
        // ⚡ Bolt: Removed unnecessary std::string allocation. std::from_chars natively supports
        // edge cases like leading/trailing decimals without needing temporary zero-padded strings.
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
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
