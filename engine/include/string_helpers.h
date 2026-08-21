// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#pragma once

#include <algorithm>
#include <cctype>
#include <charconv>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "axiom_export.h"
#include "fixed_vector.h"

namespace Utils {

// Fast string-to-double conversion using std::from_chars (C++17)
inline std::optional<double> FastParseDouble(std::string_view sv) {
  if (sv.empty()) return std::nullopt;

  std::string_view p_sv = sv;
  if (p_sv.front() == '+') {
    p_sv.remove_prefix(1);
  }

  if (p_sv.empty()) return std::nullopt;

  double result;
#if defined(__apple_build_version__) || \
    (defined(__GNUC__) && __GNUC__ < 11 && !defined(__clang__))
  // Fallback for compilers with missing floating-point from_chars
  std::string str(p_sv);
  if (str.front() == '.')
    str = "0" + str;
  else if (str.back() == '.')
    str += "0";

  try {
    size_t pos;
    result = std::stod(str, &pos);
    if (pos != str.size()) return std::nullopt;
    return result;
  } catch (...) {
    return std::nullopt;
  }
#else
  // Optimistic fast-path: zero-allocation parsing
  auto [ptr, ec] =
      std::from_chars(p_sv.data(), p_sv.data() + p_sv.size(), result);
  if (ec == std::errc{} && ptr == p_sv.data() + p_sv.size()) {
    return result;
  }

  // Handle edge cases that std::from_chars might not handle well
  std::string str(p_sv);

  // Handle leading decimal point (e.g., ".5" -> "0.5")
  if (str.front() == '.') {
    str = "0" + str;
  }
  // Handle trailing decimal point (e.g., "5." -> "5.0")
  else if (str.back() == '.') {
    str += "0";
  }

  auto [ptr2, ec2] =
      std::from_chars(str.data(), str.data() + str.size(), result);
  // Check if conversion was successful AND we consumed the entire string
  return (ec2 == std::errc{} && ptr2 == str.data() + str.size())
             ? std::optional<double>(result)
             : std::nullopt;
#endif
}

// Helper to trim strings (removes whitespace from both ends)
AXIOM_EXPORT std::string Trim(std::string_view str);

// Helper to split string by delimiter
AXIOM_EXPORT AXIOM::FixedVector<std::string, 256> Split(std::string_view s,
                                                        char delimiter);

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
AXIOM_EXPORT std::string ReplaceAll(std::string_view str, std::string_view from,
                                    std::string_view to);
}  // namespace Utils
