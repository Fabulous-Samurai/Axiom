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
        
        char stack_buf[128];
        char* buf_ptr = stack_buf;
        
        bool use_heap = sv.size() + 2 >= sizeof(stack_buf);
        std::string heap_buf;
        size_t final_size = 0;

        if (use_heap) {
            heap_buf = std::string(sv);
            if (heap_buf.front() == '.') {
                heap_buf = "0" + heap_buf;
            } else if (heap_buf.back() == '.') {
                heap_buf += "0";
            }
            buf_ptr = heap_buf.data();
            final_size = heap_buf.size();
        } else {
            size_t i = 0;
            if (sv.front() == '.') {
                stack_buf[i++] = '0';
                std::copy(sv.begin(), sv.end(), stack_buf + i);
                i += sv.size();
            } else {
                std::copy(sv.begin(), sv.end(), stack_buf + i);
                i += sv.size();
                if (sv.back() == '.') {
                    stack_buf[i++] = '0';
                }
            }
            stack_buf[i] = '\0';
            final_size = i;
        }
        
        double result;
#if defined(__apple_build_version__) || (defined(__GNUC__) && __GNUC__ < 11 && !defined(__clang__))
        // Fallback for compilers with missing floating-point from_chars
        char* end_ptr = nullptr;
        errno = 0;
        result = std::strtod(buf_ptr, &end_ptr);

        if (end_ptr != buf_ptr + final_size) {
            return std::nullopt;
        }
        if (errno == ERANGE) {
            if (result == 0.0) {
                // Underflow is accepted
            } else {
                return std::nullopt; // Overflow
            }
        }
        return result;
#else
        auto [ptr, ec] = std::from_chars(buf_ptr, buf_ptr + final_size, result);
        // Check if conversion was successful AND we consumed the entire string
        return (ec == std::errc{} && ptr == buf_ptr + final_size) ? std::optional<double>(result) : std::nullopt;
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
