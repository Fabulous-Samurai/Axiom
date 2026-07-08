// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file statistics_engine.h
 * @brief Statistical analysis and data processing capabilities
 * Adds descriptive stats, distributions, hypothesis testing, regression
 */
#pragma once

#include <algorithm>
#include <numeric>
#include <vector>

#include "axiom_export.h"
#include "dynamic_calc_types.h"

namespace AXIOM {

class AXIOM_EXPORT StatisticsEngine {
 public:
  // Descriptive Statistics
  EngineResult Mean(const Vector& data);
  EngineResult Median(Vector data);  // Note: modifies input for sorting
  EngineResult Mode(const Vector& data);
  EngineResult Variance(const Vector& data);
  EngineResult StandardDeviation(const Vector& data);
  EngineResult Skewness(const Vector& data);
  EngineResult Kurtosis(const Vector& data);

  // Percentiles and Quantiles
  EngineResult Percentile(Vector data, double p);
  EngineResult Quartiles(Vector data);
  EngineResult InterquartileRange(Vector data);

  // Correlation and Regression
  EngineResult Correlation(const Vector& x, const Vector& y);
  EngineResult LinearRegression(const Vector& x, const Vector& y);
  EngineResult RSquared(const Vector& x, const Vector& y);

  // Probability Distributions
  EngineResult NormalPDF(double x, double mean = 0, double stddev = 1);
  EngineResult NormalCDF(double x, double mean = 0, double stddev = 1);
  EngineResult TDistributionPDF(double x, double degrees_freedom);
  EngineResult ChiSquaredPDF(double x, double degrees_freedom);

  // Hypothesis Testing
  EngineResult TTest(const Vector& sample1, const Vector& sample2);
  EngineResult ChiSquaredTest(const Matrix& observed, const Matrix& expected);
  EngineResult ANOVAOneWay(const std::vector<Vector>& groups);

  // Time Series
  EngineResult MovingAverage(const Vector& data, int window_size);
  EngineResult ExponentialSmoothing(const Vector& data, double alpha);
};

}  // namespace AXIOM
