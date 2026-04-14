// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file dynamic_calc_types.h
 * @brief Core data types and result structures for AXIOM Engine.
 */

#pragma once

#include <cmath>
#include <complex>
#include <functional>
#include <limits>
#include <optional>
#include <string_view>
#include <utility>
#include <variant>

#include "axiom_status.h"
#include "fixed_vector.h"

namespace AXIOM {

// --- 1. CORE TYPES & ALIASES (Taşıyıcı Kolonlar) ---
using Number = std::variant<double, std::complex<double>>;
using Vector = AXIOM::FixedVector<double, 256>;
using Matrix = AXIOM::FixedVector<Vector, 256>;

template <typename T>
using StringUnorderedMap = FixedVector<std::pair<std::string_view, T>, 256>;

template <typename T>
using StringMap = FixedVector<std::pair<std::string_view, T>, 256>;

// --- 2. MATH UTILITIES ---
inline bool IsReal(const Number &num) noexcept {
  return std::holds_alternative<double>(num);
}
inline bool IsComplex(const Number &num) noexcept {
  return std::holds_alternative<std::complex<double>>(num);
}

inline double GetReal(const Number &num) noexcept {
  if (IsReal(num)) return std::get<double>(num);
  return std::get<std::complex<double>>(num).real();
}

inline std::complex<double> GetComplex(const Number &num) noexcept {
  if (IsReal(num)) return std::complex<double>(std::get<double>(num), 0.0);
  return std::get<std::complex<double>>(num);
}

// --- 3. ERROR TYPES (Mapped to axiom_status.h) ---
using EngineErrorResult = std::variant<CalcErr, LinAlgErr>;

// --- 3b. CALCULATION MODE ENUM ---
enum class CalculationMode {
  ALGEBRAIC,
  LINEAR_SYSTEM,
  STATISTICS,
  SYMBOLIC,
  UNITS,
  PLOT,
  PYTHON
};

// --- 3c. OPERATOR PRECEDENCE ENUM (used by AlgebraicParser) ---
enum class Precedence {
  None = 0,
  AddSub = 1,
  MultiDiv = 2,
  Pow = 3,
  Unary = 4
};

// --- 4. THE TITANIUM RESULT STRUCT (Move Semantics) ---
struct EngineResult {
  std::optional<std::variant<double, std::complex<double>, AXIOM::Number,
                             Vector, Matrix, std::string_view>>
      result = std::nullopt;
  std::optional<EngineErrorResult> error = std::nullopt;

  EngineResult() = default;

  // Zero-copy move operations (noexcept guarantees safe vector reallocations)
  EngineResult(EngineResult &&other) noexcept = default;
  EngineResult &operator=(EngineResult &&other) noexcept = default;

  // Fallback copy operations
  EngineResult(const EngineResult &other) = default;
  EngineResult &operator=(const EngineResult &other) = default;

  std::optional<double> GetDouble() const noexcept {
    if (!result.has_value()) return std::nullopt;
    return std::visit(
        [](const auto &val) -> std::optional<double> {
          using T = std::decay_t<decltype(val)>;
          if constexpr (std::is_same_v<T, double>)
            return val;
          else if constexpr (std::is_same_v<T, std::complex<double>>)
            return val.real();
          else if constexpr (std::is_same_v<T, AXIOM::Number>)
            return AXIOM::GetReal(val);
          else
            return std::nullopt;
        },
        result.value());
  }

  std::optional<std::complex<double>> GetComplex() const noexcept {
    if (!result.has_value()) return std::nullopt;
    return std::visit(
        [](const auto &val) -> std::optional<std::complex<double>> {
          using T = std::decay_t<decltype(val)>;
          if constexpr (std::is_same_v<T, double>)
            return std::complex<double>(val, 0.0);
          else if constexpr (std::is_same_v<T, std::complex<double>>)
            return val;
          else if constexpr (std::is_same_v<T, AXIOM::Number>)
            return AXIOM::GetComplex(val);
          else
            return std::nullopt;
        },
        result.value());
  }

  std::optional<Vector> GetVector() const noexcept {
    if (!result.has_value()) return std::nullopt;
    if (auto *v = std::get_if<Vector>(&result.value())) return *v;
    return std::nullopt;
  }

  std::optional<Matrix> GetMatrix() const noexcept {
    if (!result.has_value()) return std::nullopt;
    if (auto *m = std::get_if<Matrix>(&result.value())) return *m;
    return std::nullopt;
  }

  bool HasResult() const noexcept {
    return result.has_value() && !error.has_value();
  }
  bool HasErrors() const noexcept { return error.has_value(); }
  bool HasValue() const noexcept { return result.has_value(); }
};

// --- 5. ZERO-COPY FACTORY FUNCTIONS ---
inline EngineResult CreateSuccessResult(Vector &&value) noexcept {
  EngineResult res;
  res.result = std::move(value);
  return res;
}

inline EngineResult CreateSuccessResult(Matrix &&value) noexcept {
  EngineResult res;
  res.result = std::move(value);
  return res;
}

inline EngineResult CreateSuccessResult(std::string_view value) noexcept {
  EngineResult res;
  res.result = value;
  return res;
}

// --- 6. FALLBACK COPY FACTORY FUNCTIONS ---
inline EngineResult CreateSuccessResult(double value) noexcept {
  EngineResult res;
  res.result = value;
  return res;
}

inline EngineResult CreateSuccessResult(
    const std::complex<double> &value) noexcept {
  EngineResult res;
  res.result =
      value;  // store as std::complex<double> directly, not wrapped in Number
  return res;
}

inline EngineResult CreateSuccessResult(const AXIOM::Number &value) noexcept {
  EngineResult res;
  if (AXIOM::IsComplex(value))
    res.result = AXIOM::GetComplex(value);
  else
    res.result = AXIOM::GetReal(value);
  return res;
}

inline EngineResult CreateSuccessResult(const Vector &value) noexcept {
  EngineResult res;
  res.result = value;
  return res;
}

inline EngineResult CreateSuccessResult(const Matrix &value) noexcept {
  EngineResult res;
  res.result = value;
  return res;
}

// --- 7. ERROR FACTORY FUNCTIONS ---
inline EngineResult CreateErrorResult(CalcErr err) noexcept {
  EngineResult res;
  res.error = EngineErrorResult{err};
  return res;
}

inline EngineResult CreateErrorResult(LinAlgErr err) noexcept {
  EngineResult res;
  res.error = EngineErrorResult{err};
  return res;
}

// Özel Linear Algebra Sonuç Tipi
struct LinAlgResult {
  std::optional<Vector> solution;
  LinAlgErr error;
};

}  // namespace AXIOM

// ---------------------------------------------------------------------------
// Backward-compatibility: bring core AXIOM types into global scope.
// Many headers reference these without AXIOM:: qualification (pre-namespace
// refactor). These declarations let them compile without mass edits.
// ---------------------------------------------------------------------------
using AXIOM::CalcErr;
using AXIOM::CalculationMode;
using AXIOM::ComputeEngine;
using AXIOM::CreateErrorResult;
using AXIOM::CreateSuccessResult;
using AXIOM::EngineErrorResult;
using AXIOM::EngineResult;
using AXIOM::GetComplex;
using AXIOM::GetReal;
using AXIOM::IsComplex;
using AXIOM::IsReal;
using AXIOM::LinAlgErr;
using AXIOM::LinAlgResult;
using AXIOM::Matrix;
using AXIOM::Number;
using AXIOM::OperationComplexity;
using AXIOM::Precedence;
using AXIOM::Vector;
