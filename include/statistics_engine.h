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
    EngineResult Mean(const Vector& data);
    EngineResult Median(const Vector& data);
    EngineResult Mode(const Vector& data);
    EngineResult Variance(const Vector& data);
    EngineResult StandardDeviation(const Vector& data);

    // Percentiles and Quantiles  
    EngineResult Percentile(const Vector& data, double p);
    
    // Correlation and Regression
    EngineResult Correlation(const Vector& x, const Vector& y);
    EngineResult LinearRegression(const Vector& x, const Vector& y);
    
    // Time Series
    EngineResult MovingAverage(const Vector& data, int window_size);
};
