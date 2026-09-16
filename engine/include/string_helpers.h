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
        // Fallback for compilers with missing floating-point from_chars

        // Zero-allocation fallback using stack buffer, with dynamic fallback for very long strings
        constexpr size_t MAX_STACK_BUF = 256;
        char stack_buf[MAX_STACK_BUF];
        std::string heap_buf;
        const char* str_ptr = nullptr;

        if (sv.size() < MAX_STACK_BUF) {
            std::copy(sv.begin(), sv.end(), stack_buf);
            stack_buf[sv.size()] = '\0';
            str_ptr = stack_buf;
        } else {
            heap_buf = std::string(sv);
            str_ptr = heap_buf.c_str();
        }

        char* end_ptr = nullptr;
        errno = 0;
        result = std::strtod(str_ptr, &end_ptr);

        // Handle error cases properly (underflow shouldn't necessarily fail parsing if it's treated as 0.0)
        if (end_ptr != str_ptr + sv.size()) return std::nullopt;
        if (errno == ERANGE && (result != 0.0 && result != -0.0)) return std::nullopt; // underflow to 0.0 is ok, overflow is not

        return result;
#else
        // Modern C++17 from_chars natively supports '.5' and '5.' forms without string copies
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
