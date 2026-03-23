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

class AXIOM_EXPORT StatisticsEngine {
public:
    // Descriptive Statistics
    AXIOM::EngineResult Mean(const AXIOM::Vector& data);
    AXIOM::EngineResult Median(const AXIOM::Vector& data);
    AXIOM::EngineResult Mode(const AXIOM::Vector& data);
    AXIOM::EngineResult Variance(const AXIOM::Vector& data);
    AXIOM::EngineResult StandardDeviation(const AXIOM::Vector& data);
    AXIOM::EngineResult Skewness(const AXIOM::Vector& data);
    AXIOM::EngineResult Kurtosis(const AXIOM::Vector& data);

    // Percentiles and Quantiles  
    AXIOM::EngineResult Percentile(const AXIOM::Vector& data, double p);
    AXIOM::EngineResult Quartiles(const AXIOM::Vector& data);
    AXIOM::EngineResult InterquartileRange(const AXIOM::Vector& data);
    
    // Correlation and Regression
    AXIOM::EngineResult Correlation(const AXIOM::Vector& x, const AXIOM::Vector& y);
    AXIOM::EngineResult LinearRegression(const AXIOM::Vector& x, const AXIOM::Vector& y);
    AXIOM::EngineResult RSquared(const AXIOM::Vector& x, const AXIOM::Vector& y);

    // Probability Distributions
    AXIOM::EngineResult NormalPDF(double x, double mean = 0, double stddev = 1);
    AXIOM::EngineResult NormalCDF(double x, double mean = 0, double stddev = 1);
    AXIOM::EngineResult TDistributionPDF(double x, double degrees_freedom);
    AXIOM::EngineResult ChiSquaredPDF(double x, double degrees_freedom);

    // Hypothesis Testing
    AXIOM::EngineResult TTest(const AXIOM::Vector& sample1, const AXIOM::Vector& sample2);
    AXIOM::EngineResult ChiSquaredTest(const AXIOM::Matrix& observed, const AXIOM::Matrix& expected);
    AXIOM::EngineResult ANOVAOneWay(const std::vector<AXIOM::Vector>& groups);
    
    // Time Series
    AXIOM::EngineResult MovingAverage(const AXIOM::Vector& data, int window_size);
    AXIOM::EngineResult ExponentialSmoothing(const AXIOM::Vector& data, double alpha);
};
