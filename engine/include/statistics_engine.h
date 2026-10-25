// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file statistics_engine.h
 * @brief Statistical analysis and data processing capabilities
 * Adds descriptive stats, distributions, hypothesis testing, regression
 */
#pragma once

#include "dynamic_calc_types.h"
#include <algorithm>
#include <numeric>
#include <vector>

#include "axiom_export.h"

namespace AXIOM {

class AXIOM_EXPORT StatisticsEngine {
public:
    // Descriptive Statistics
    EngineResult Mean(const Vector& data) const;
    EngineResult Median(Vector data) const;  // Note: modifies input for sorting
    EngineResult Mode(Vector data) const;
    EngineResult Variance(const Vector& data) const;
    EngineResult StandardDeviation(const Vector& data) const;
    EngineResult Skewness(const Vector& data) const;
    EngineResult Kurtosis(const Vector& data) const;
    
    // Percentiles and Quantiles  
    EngineResult Percentile(Vector data, double p) const;
    EngineResult Quartiles(Vector data) const;
    EngineResult InterquartileRange(Vector data) const;
    
    // Correlation and Regression
    EngineResult Correlation(const Vector& x, const Vector& y) const;
    EngineResult LinearRegression(const Vector& x, const Vector& y) const;
    EngineResult RSquared(const Vector& x, const Vector& y) const;
    
    // Probability Distributions
    EngineResult NormalPDF(double x, double mean = 0, double stddev = 1) const;
    EngineResult NormalCDF(double x, double mean = 0, double stddev = 1) const;
    EngineResult TDistributionPDF(double x, double degrees_freedom) const;
    EngineResult ChiSquaredPDF(double x, double degrees_freedom) const;
    
    // Hypothesis Testing
    EngineResult TTest(const Vector& sample1, const Vector& sample2) const;
    EngineResult ChiSquaredTest(const Matrix& observed, const Matrix& expected) const;
    EngineResult ANOVAOneWay(const std::vector<Vector>& groups) const;
    
    // Time Series
    EngineResult MovingAverage(const Vector& data, int window_size) const;
    EngineResult ExponentialSmoothing(const Vector& data, double alpha) const;
};

} // namespace AXIOM
