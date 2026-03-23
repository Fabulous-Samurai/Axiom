#include "statistics_engine.h"
#include <algorithm>
#include <cmath>
#include <map>

namespace AXIOM {

EngineResult StatisticsEngine::Mean(const Vector& data) {
    if (data.empty()) return CreateErrorResult(CalcErr::ArgumentMismatch);
    
    double sum = 0.0;
    for (double val : data) {
        if (!std::isfinite(val)) return CreateErrorResult(CalcErr::DomainError);
        sum += val;
    }
    return CreateSuccessResult(sum / data.size());
}

EngineResult StatisticsEngine::Median(const Vector& data) {
    if (data.empty()) return CreateErrorResult(CalcErr::ArgumentMismatch);

    Vector local_data = data;
    auto n = local_data.size();
    auto mid = n / 2;
    
    if (n % 2 == 0) {
        std::nth_element(local_data.begin(), local_data.begin() + mid, local_data.end());
        double val1 = local_data[mid];
        std::nth_element(local_data.begin(), local_data.begin() + mid - 1, local_data.begin() + mid);
        double val2 = local_data[mid - 1];
        return CreateSuccessResult((val1 + val2) / 2.0);
    } else {
        std::nth_element(local_data.begin(), local_data.begin() + mid, local_data.end());
        return CreateSuccessResult(local_data[mid]);
    }
}

EngineResult StatisticsEngine::Mode(const Vector& data) {
    if (data.empty()) return CreateErrorResult(CalcErr::ArgumentMismatch);
    
    std::map<double, int> frequency;
    for (double val : data) {
        frequency[val]++;
    }
    
    double mode_val = data[0];
    int max_count = 0;
    for (const auto& [val, count] : frequency) {
        if (count > max_count) {
            max_count = count;
            mode_val = val;
        }
    }
    
    return CreateSuccessResult(mode_val);
}

EngineResult StatisticsEngine::Variance(const Vector& data) {
    if (data.size() < 2) return CreateErrorResult(CalcErr::ArgumentMismatch);
    
    auto mean_result = Mean(data);
    if (!mean_result.result.has_value()) return mean_result;

    // Use the EngineResult helper to extract a double regardless of underlying variant
    auto mean_val_opt = mean_result.GetDouble();
    if (!mean_val_opt.has_value()) return CreateErrorResult(CalcErr::ArgumentMismatch);
    double mean_val = *mean_val_opt;

    double sum_sq_diff = 0.0;
    
    for (double val : data) {
        double diff = val - mean_val;
        sum_sq_diff += diff * diff;
    }
    
    return CreateSuccessResult(sum_sq_diff / (data.size() - 1));
}

EngineResult StatisticsEngine::StandardDeviation(const Vector& data) {
    auto var_result = Variance(data);
    if (!var_result.result.has_value()) return var_result;
    
    auto variance_opt = var_result.GetDouble();
    if (!variance_opt.has_value()) return CreateErrorResult(CalcErr::ArgumentMismatch);
    double variance = *variance_opt;
    return CreateSuccessResult(std::sqrt(variance));
}

EngineResult StatisticsEngine::Correlation(const Vector& x, const Vector& y) {
    if (x.size() != y.size() || x.size() < 2) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }
    
    auto x_mean_result = Mean(x);
    auto y_mean_result = Mean(y);
    if (!x_mean_result.result.has_value() || !y_mean_result.result.has_value()) {
        return CreateErrorResult(CalcErr::DomainError);
    }

    auto x_mean_opt = x_mean_result.GetDouble();
    auto y_mean_opt = y_mean_result.GetDouble();
    if (!x_mean_opt.has_value() || !y_mean_opt.has_value()) {
        return CreateErrorResult(CalcErr::DomainError);
    }
    double x_mean = *x_mean_opt;
    double y_mean = *y_mean_opt;

    double numerator = 0.0, sum_x_sq = 0.0, sum_y_sq = 0.0;

    for (size_t i = 0; i < x.size(); ++i) {
        double x_diff = x[i] - x_mean;
        double y_diff = y[i] - y_mean;
        numerator += x_diff * y_diff;
        sum_x_sq += x_diff * x_diff;
        sum_y_sq += y_diff * y_diff;
    }
    
    double denominator = std::sqrt(sum_x_sq * sum_y_sq);
    if (denominator == 0.0) return CreateErrorResult(CalcErr::DivideByZero);
    
    return CreateSuccessResult(numerator / denominator);
}

EngineResult StatisticsEngine::LinearRegression(const Vector& x, const Vector& y) {
    if (x.size() != y.size() || x.size() < 2) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }
    
    auto x_mean_result = Mean(x);
    auto y_mean_result = Mean(y);
    if (!x_mean_result.result.has_value() || !y_mean_result.result.has_value()) {
        return CreateErrorResult(CalcErr::DomainError);
    }

    auto x_mean_opt2 = x_mean_result.GetDouble();
    auto y_mean_opt2 = y_mean_result.GetDouble();
    if (!x_mean_opt2.has_value() || !y_mean_opt2.has_value()) {
        return CreateErrorResult(CalcErr::DomainError);
    }
    double x_mean = *x_mean_opt2;
    double y_mean = *y_mean_opt2;

    double numerator = 0.0, denominator = 0.0;
    
    for (size_t i = 0; i < x.size(); ++i) {
        double x_diff = x[i] - x_mean;
        numerator += x_diff * (y[i] - y_mean);
        denominator += x_diff * x_diff;
    }
    
    if (denominator == 0.0) return CreateErrorResult(CalcErr::DivideByZero);
    
    double slope = numerator / denominator;
    double intercept = y_mean - slope * x_mean;
    
    // Return [slope, intercept]
    return CreateSuccessResult(Vector{slope, intercept});
}

EngineResult StatisticsEngine::Percentile(const Vector& data, double p) {
    if (data.empty() || p < 0 || p > 100) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }
    
    Vector local_data = data;
    auto n = local_data.size();

    if (p == 0) {
        auto it = std::min_element(local_data.begin(), local_data.end());
        return CreateSuccessResult(*it);
    }
    if (p == 100) {
        auto it = std::max_element(local_data.begin(), local_data.end());
        return CreateSuccessResult(*it);
    }
    
    double index = (p / 100.0) * (n - 1);
    size_t lower = static_cast<size_t>(index);
    size_t upper = lower + 1;
    
    if (upper >= n) {
        auto it = std::max_element(local_data.begin(), local_data.end());
        return CreateSuccessResult(*it);
    }
    
    std::nth_element(local_data.begin(), local_data.begin() + lower, local_data.end());
    double val_lower = local_data[lower];
    std::nth_element(local_data.begin() + upper, local_data.begin() + upper, local_data.end());
    double val_upper = local_data[upper];

    double weight = index - lower;
    double result = val_lower * (1.0 - weight) + val_upper * weight;
    
    return CreateSuccessResult(result);
}

EngineResult StatisticsEngine::Quartiles(const Vector& data) {
    if (data.empty()) return CreateErrorResult(CalcErr::ArgumentMismatch);

    auto q1 = Percentile(data, 25.0);
    auto q2 = Percentile(data, 50.0);
    auto q3 = Percentile(data, 75.0);

    if (!q1.HasResult() || !q2.HasResult() || !q3.HasResult()) {
        return CreateErrorResult(CalcErr::DomainError);
    }

    Vector res = {*q1.GetDouble(), *q2.GetDouble(), *q3.GetDouble()};
    return CreateSuccessResult(std::move(res));
}

EngineResult StatisticsEngine::InterquartileRange(const Vector& data) {
    auto q = Quartiles(data);
    if (!q.HasResult()) return q;

    if (auto val = q.result.value(); std::holds_alternative<Vector>(val)) {
        const auto& v = std::get<Vector>(val);
        if (v.size() == 3) {
            return CreateSuccessResult(v[2] - v[0]);
        }
    }
    return CreateErrorResult(CalcErr::DomainError);
}

EngineResult StatisticsEngine::MovingAverage(const Vector& data, int window_size) {
    if (data.empty() || window_size <= 0 || window_size > static_cast<int>(data.size())) {
        return CreateErrorResult(CalcErr::ArgumentMismatch);
    }
    
    Vector result;
    result.reserve(data.size() - window_size + 1);
    
    for (size_t i = 0; i <= data.size() - window_size; ++i) {
        double sum = 0.0;
        for (int j = 0; j < window_size; ++j) {
            sum += data[i + j];
        }
        result.push_back(sum / window_size);
    }
    
    return CreateSuccessResult(result);
}

// Stubs for remaining public API to ensure compilation.
EngineResult StatisticsEngine::Skewness(const Vector&) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult StatisticsEngine::Kurtosis(const Vector&) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult StatisticsEngine::RSquared(const Vector&, const Vector&) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult StatisticsEngine::NormalPDF(double, double, double) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult StatisticsEngine::NormalCDF(double, double, double) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult StatisticsEngine::TDistributionPDF(double, double) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult StatisticsEngine::ChiSquaredPDF(double, double) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult StatisticsEngine::TTest(const Vector&, const Vector&) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult StatisticsEngine::ChiSquaredTest(const Matrix&, const Matrix&) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult StatisticsEngine::ANOVAOneWay(const std::vector<Vector>&) { return CreateErrorResult(CalcErr::OperationNotFound); }
EngineResult StatisticsEngine::ExponentialSmoothing(const Vector&, double) { return CreateErrorResult(CalcErr::OperationNotFound); }
} // namespace AXIOM
