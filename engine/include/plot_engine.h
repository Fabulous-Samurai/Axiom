// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file plot_engine.h
 * @brief Plotting and visualization capabilities using ASCII art
 * Generates terminal-based plots for functions and data
 */
#pragma once

#include <functional>

#include "arena.h"
#include "dynamic_calc_types.h"

namespace AXIOM {

struct alignas(64) PlotConfig {
  int width = 80;
  int height = 20;
  double x_min = -10;
  double x_max = 10;
  double y_min = -5;
  double y_max = 5;
  bool show_grid = true;
  bool show_axes = true;
  char plot_char = '*';
};

#include "axiom_export.h"

class AXIOM_EXPORT PlotEngine {
 private:
  Arena arena_;

 public:
  explicit PlotEngine() noexcept : arena_(1024 * 512) {}

  // Function plotting
  std::string_view PlotFunction(std::string_view expression,
                                const PlotConfig& config = {}) noexcept;
  std::string_view PlotParametric(std::string_view x_expr,
                                  std::string_view y_expr, double t_min,
                                  double t_max,
                                  const PlotConfig& config = {}) noexcept;

  // Data generation for Python plotting
  // Samples the function over [x_min, x_max] returning rows of [x, y]
  Matrix ComputeFunctionData(std::string_view expression,
                             const PlotConfig& config = {},
                             int samples = -1) noexcept;

  // Data plotting
  std::string_view PlotData(const Vector& x_data, const Vector& y_data,
                            const PlotConfig& config = {}) noexcept;
  std::string_view Histogram(const Vector& data, int bins = 10,
                             const PlotConfig& config = {}) noexcept;
  std::string_view BoxPlot(const Vector& data,
                           const PlotConfig& config = {}) noexcept;

  // 3D Surface plots (top-down view)
  std::string_view PlotSurface(std::string_view expression,
                               const PlotConfig& config = {}) noexcept;

  // Specialized plots
  std::string_view PlotComplex(const Vector& real_parts,
                               const Vector& imag_parts,
                               const PlotConfig& config = {}) noexcept;
  std::string_view PolarPlot(std::string_view r_expression,
                             const PlotConfig& config = {}) noexcept;

 private:
  char GetCharForValue(double value, double min_val, double max_val) noexcept;
  std::pair<int, int> MapToScreen(double x, double y,
                                  const PlotConfig& config) noexcept;
};

}  // namespace AXIOM
