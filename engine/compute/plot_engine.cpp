// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "plot_engine.h"
#include "algebraic_parser.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdarg>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace AXIOM {

/**
 * @brief Internal helper to append characters to a FixedVector
 */
template<size_t N>
static void AppendChars(FixedVector<char, N>& buffer, std::string_view sv) noexcept {
    for (const char c : sv) {
        if (buffer.size() < buffer.capacity()) {
            buffer.push_back(c);
        }
    }
}

/**
 * @brief Internal helper to format strings into a FixedVector
 */
template<size_t N>
static void FormatChars(FixedVector<char, N>& buffer, const char* format, ...) noexcept {
    char temp[128];
    va_list args;
    va_start(args, format);
    const int len = vsnprintf(temp, sizeof(temp), format, args);
    va_end(args);
    if (len > 0) {
        AppendChars(buffer, std::string_view(temp, static_cast<size_t>(len)));
    }
}

char PlotEngine::GetCharForValue(double value, double min_val, double max_val) noexcept {
    const char intensity[] = " .:-=+*#%@";
    const int num_levels = static_cast<int>(sizeof(intensity)) - 1;
    if (!std::isfinite(value)) return ' ';
    if (value <= min_val) return intensity[0];
    if (value >= max_val) return intensity[num_levels - 1];
    
    const double range = max_val - min_val;
    if (std::abs(range) < 1e-12) return intensity[num_levels / 2];
    
    const int level = static_cast<int>((value - min_val) / range * (num_levels - 1));
    return intensity[std::clamp(level, 0, num_levels - 1)];
}

std::pair<int, int> PlotEngine::MapToScreen(double x, double y, const PlotConfig& config) noexcept {
    const double x_range = config.x_max - config.x_min;
    const double y_range = config.y_max - config.y_min;
    
    // Arithmetic traps: handle zero range to avoid division by zero
    const double safe_x_range = (std::abs(x_range) < 1e-12) ? 1.0 : x_range;
    const double safe_y_range = (std::abs(y_range) < 1e-12) ? 1.0 : y_range;

    const int w = std::max(1, config.width);
    const int h = std::max(1, config.height);

    const int screen_x = static_cast<int>((x - config.x_min) / safe_x_range * (w - 1));
    const int screen_y = static_cast<int>((config.y_max - y) / safe_y_range * (h - 1));
    
    return {screen_x, screen_y};
}

std::string_view PlotEngine::PlotFunction(std::string_view expression, const PlotConfig& config) noexcept {
    arena_.reset();
    AlgebraicParser parser;
    const int width = std::clamp(config.width, 1, 256);
    const int height = std::clamp(config.height, 1, 256);

    // Stack-allocated grid buffer (Zenith Pillar 1)
    char grid[65536]; 
    std::memset(grid, ' ', static_cast<size_t>(width) * height);

    const double x_min = config.x_min;
    const double x_max = config.x_max;
    const double y_min = config.y_min;
    const double y_max = config.y_max;
    
    const double x_range = x_max - x_min;
    const double x_step = (std::abs(x_range) > 1e-12) ? (x_range / (width - 1)) : 0.0;

    for (int col = 0; col < width; ++col) {
        const double x = x_min + col * x_step;
        SymbolTable context;
        context.push_back({"x", Number(x)});

        const auto result = parser.ParseAndExecuteWithContext(expression, context);
        if (const auto y_opt = result.GetDouble(); y_opt.has_value()) {
            const double y = *y_opt;
            if (std::isfinite(y) && y >= y_min && y <= y_max) {
                const auto [screen_x, screen_y] = MapToScreen(x, y, config);
                if (screen_x >= 0 && screen_x < width && screen_y >= 0 && screen_y < height) {
                    grid[screen_y * width + screen_x] = config.plot_char;
                }
            }
        }
    }

    // Draw Axes
    if (config.show_axes) {
        if (0 >= x_min && 0 <= x_max) {
            const auto [axis_x, _] = MapToScreen(0, 0, config);
            if (axis_x >= 0 && axis_x < width) {
                for (int row = 0; row < height; ++row) {
                    if (grid[row * width + axis_x] == ' ') grid[row * width + axis_x] = '|';
                }
            }
        }
        if (0 >= y_min && 0 <= y_max) {
            const auto [_, axis_y] = MapToScreen(0, 0, config);
            if (axis_y >= 0 && axis_y < height) {
                for (int col = 0; col < width; ++col) {
                    if (grid[axis_y * width + col] == ' ') grid[axis_y * width + col] = '-';
                }
            }
        }
    }

    // Build final result in FixedVector and copy to Arena
    FixedVector<char, 70000> result_buffer;
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            result_buffer.push_back(grid[row * width + col]);
        }
        result_buffer.push_back('\n');
    }

    return arena_.allocString(std::string_view(result_buffer.data(), result_buffer.size()));
}

std::string_view PlotEngine::PlotParametric(std::string_view x_expr, std::string_view y_expr, 
                                           double t_min, double t_max, const PlotConfig& config) noexcept {
    arena_.reset();
    AlgebraicParser parser;
    const int width = std::clamp(config.width, 1, 256);
    const int height = std::clamp(config.height, 1, 256);

    char grid[65536];
    std::memset(grid, ' ', static_cast<size_t>(width) * height);

    const int samples = 512;
    const double t_range = t_max - t_min;
    const double t_step = (std::abs(t_range) > 1e-12) ? (t_range / (samples - 1)) : 0.0;

    for (int i = 0; i < samples; ++i) {
        const double t = t_min + i * t_step;
        SymbolTable context;
        context.push_back({"t", Number(t)});

        const auto res_x = parser.ParseAndExecuteWithContext(x_expr, context);
        const auto res_y = parser.ParseAndExecuteWithContext(y_expr, context);
        
        if (const auto x_opt = res_x.GetDouble(); x_opt) {
            if (const auto y_opt = res_y.GetDouble(); y_opt) {
                const double x = *x_opt;
                const double y = *y_opt;
                if (std::isfinite(x) && std::isfinite(y) &&
                    x >= config.x_min && x <= config.x_max &&
                    y >= config.y_min && y <= config.y_max) {
                    const auto [screen_x, screen_y] = MapToScreen(x, y, config);
                    if (screen_x >= 0 && screen_x < width && screen_y >= 0 && screen_y < height) {
                        grid[screen_y * width + screen_x] = config.plot_char;
                    }
                }
            }
        }
    }

    if (config.show_axes) {
        const auto [ax, ay] = MapToScreen(0, 0, config);
        if (ax >= 0 && ax < width) for (int r = 0; r < height; ++r) if (grid[r * width + ax] == ' ') grid[r * width + ax] = '|';
        if (ay >= 0 && ay < height) for (int c = 0; c < width; ++c) if (grid[ay * width + c] == ' ') grid[ay * width + c] = '-';
    }

    FixedVector<char, 70000> result_buffer;
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) result_buffer.push_back(grid[row * width + col]);
        result_buffer.push_back('\n');
    }
    return arena_.allocString(std::string_view(result_buffer.data(), result_buffer.size()));
}

Matrix PlotEngine::ComputeFunctionData(std::string_view expression, const PlotConfig& config, int samples) noexcept {
    AlgebraicParser parser;
    Matrix data;
    const int n = (samples > 0) ? std::min(samples, 256) : std::max(2, std::min(config.width, 256));
    const double x_range = config.x_max - config.x_min;
    const double x_step = (n > 1) ? (x_range / (n - 1)) : 0.0;

    for (int i = 0; i < n; ++i) {
        const double x = config.x_min + i * x_step;
        SymbolTable context;
        context.push_back({"x", Number(x)});

        const auto result = parser.ParseAndExecuteWithContext(expression, context);
        if (const auto y_opt = result.GetDouble(); y_opt.has_value()) {
            const double y = *y_opt;
            if (std::isfinite(y)) {
                Vector point;
                point.push_back(x);
                point.push_back(y);
                data.push_back(std::move(point));
            }
        }
    }
    return data;
}

std::string_view PlotEngine::PlotData(const Vector& x_data, const Vector& y_data, const PlotConfig& config) noexcept {
    arena_.reset();
    if (x_data.size() != y_data.size() || x_data.empty()) {
        return arena_.allocString("Error: Data vectors must be same size and non-empty\n");
    }

    const int width = std::clamp(config.width, 1, 256);
    const int height = std::clamp(config.height, 1, 256);

    char grid[65536];
    std::memset(grid, ' ', static_cast<size_t>(width) * height);

    for (size_t i = 0; i < x_data.size(); ++i) {
        const double x = x_data[i];
        const double y = y_data[i];
        
        if (std::isfinite(x) && std::isfinite(y) && 
            x >= config.x_min && x <= config.x_max &&
            y >= config.y_min && y <= config.y_max) {
            
            const auto [screen_x, screen_y] = MapToScreen(x, y, config);
            if (screen_x >= 0 && screen_x < width && screen_y >= 0 && screen_y < height) {
                grid[screen_y * width + screen_x] = config.plot_char;
            }
        }
    }

    FixedVector<char, 70000> result_buffer;
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) result_buffer.push_back(grid[row * width + col]);
        result_buffer.push_back('\n');
    }
    return arena_.allocString(std::string_view(result_buffer.data(), result_buffer.size()));
}

std::string_view PlotEngine::Histogram(const Vector& data, int bins, const PlotConfig& config) noexcept {
    arena_.reset();
    if (data.empty() || bins <= 0) {
        return arena_.allocString("Error: Data must be non-empty and bins > 0\n");
    }
    
    double data_min = data[0];
    double data_max = data[0];
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i] < data_min) data_min = data[i];
        if (data[i] > data_max) data_max = data[i];
    }
    
    if (std::abs(data_max - data_min) < 1e-12) {
        return arena_.allocString("Error: All data points are identical\n");
    }
    
    const int actual_bins = std::clamp(bins, 1, 256);
    FixedVector<int, 256> hist;
    hist.assign(actual_bins, 0);
    const double bin_width = (data_max - data_min) / actual_bins;
    
    for (const double val : data) {
        int bin = static_cast<int>((val - data_min) / bin_width);
        if (bin >= actual_bins) bin = actual_bins - 1;
        if (bin < 0) bin = 0;
        hist[bin]++;
    }
    
    int max_freq = 0;
    for (int i = 0; i < actual_bins; ++i) {
        if (hist[i] > max_freq) max_freq = hist[i];
    }
    
    if (max_freq == 0) return arena_.allocString("Error: No valid data\n");
    
    FixedVector<char, 32768> result_buffer;
    FormatChars(result_buffer, "Histogram (%zu points, %d bins):\n", data.size(), actual_bins);
    
    for (int i = actual_bins - 1; i >= 0; --i) {
        const double bin_start = data_min + i * bin_width;
        const double bin_end = bin_start + bin_width;
        const int bar_length = static_cast<int>((static_cast<double>(hist[i]) * config.width) / max_freq);
        
        FormatChars(result_buffer, "[%7.2f - %7.2f) ", bin_start, bin_end);
        for (int j = 0; j < bar_length; ++j) result_buffer.push_back(config.plot_char);
        FormatChars(result_buffer, " (%d)\n", hist[i]);
    }
    
    return arena_.allocString(std::string_view(result_buffer.data(), result_buffer.size()));
}

std::string_view PlotEngine::BoxPlot(const Vector& data, const PlotConfig& config) noexcept {
    arena_.reset();
    if (data.empty()) return arena_.allocString("Error: Data is empty\n");

    Vector sorted = data;
    const size_t n = sorted.size();

    // Find min and max
    auto minmax = std::minmax_element(sorted.begin(), sorted.end());
    const double min_val = *minmax.first;
    const double max_val = *minmax.second;

    // Find median
    const size_t med_idx = n / 2;
    std::nth_element(sorted.begin(), sorted.begin() + med_idx, sorted.end());
    const double median = sorted[med_idx];

    // Find Q1
    const size_t q1_idx = n / 4;
    if (q1_idx < med_idx) {
        std::nth_element(sorted.begin(), sorted.begin() + q1_idx, sorted.begin() + med_idx);
    }
    const double q1 = sorted[q1_idx];

    // Find Q3
    const size_t q3_idx = 3 * n / 4;
    if (q3_idx > med_idx && q3_idx < n) {
        std::nth_element(sorted.begin() + med_idx + 1, sorted.begin() + q3_idx, sorted.end());
    }
    const double q3 = sorted[q3_idx];

    const int width = std::clamp(config.width, 10, 256);
    const double range = max_val - min_val;
    const double safe_range = (std::abs(range) < 1e-12) ? 1.0 : range;

    auto map_x = [&](double val) -> int {
        return static_cast<int>((val - min_val) / safe_range * (width - 1));
    };

    FixedVector<char, 1024> line1; line1.assign(width, ' ');
    line1[map_x(min_val)] = '|'; line1[map_x(max_val)] = '|';
    for (int i = map_x(min_val) + 1; i < map_x(max_val); ++i) line1[i] = '-';

    FixedVector<char, 1024> line2; line2.assign(width, ' ');
    line2[map_x(q1)] = '['; line2[map_x(q3)] = ']'; line2[map_x(median)] = '|';
    for (int i = map_x(q1) + 1; i < map_x(q3); ++i) if (i != map_x(median)) line2[i] = '=';

    FixedVector<char, 4096> result;
    AppendChars(result, "Box Plot:\n");
    AppendChars(result, std::string_view(line1.data(), line1.size())); AppendChars(result, "\n");
    AppendChars(result, std::string_view(line2.data(), line2.size())); AppendChars(result, "\n");
    AppendChars(result, std::string_view(line1.data(), line1.size())); AppendChars(result, "\n");
    FormatChars(result, "Min: %.2f, Q1: %.2f, Median: %.2f, Q3: %.2f, Max: %.2f\n", min_val, q1, median, q3, max_val);

    return arena_.allocString(std::string_view(result.data(), result.size()));
}

std::string_view PlotEngine::PlotSurface(std::string_view expression, const PlotConfig& config) noexcept {
    arena_.reset();
    AlgebraicParser parser;
    const int width = std::clamp(config.width, 1, 256);
    const int height = std::clamp(config.height, 1, 256);
    
    char grid[65536];
    const double x_min = config.x_min;
    const double x_max = config.x_max;
    const double y_min = config.x_min; // Use symmetric domain for y input if not specified
    const double y_max = config.x_max;
    
    const double x_step = (width > 1) ? (x_max - x_min) / (width - 1) : 0;
    const double y_step = (height > 1) ? (y_max - y_min) / (height - 1) : 0;

    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            const double x = x_min + c * x_step;
            const double y = y_max - r * y_step;
            SymbolTable context;
            context.push_back({"x", Number(x)});
            context.push_back({"y", Number(y)});
            
            const auto res = parser.ParseAndExecuteWithContext(expression, context);
            if (const auto z_opt = res.GetDouble(); z_opt) {
                grid[r * width + c] = GetCharForValue(*z_opt, config.y_min, config.y_max);
            } else {
                grid[r * width + c] = ' ';
            }
        }
    }
    
    FixedVector<char, 70000> result_buffer;
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) result_buffer.push_back(grid[row * width + col]);
        result_buffer.push_back('\n');
    }
    return arena_.allocString(std::string_view(result_buffer.data(), result_buffer.size()));
}

std::string_view PlotEngine::PlotComplex(const Vector& real_parts, const Vector& imag_parts, const PlotConfig& config) noexcept {
    // Treat real as X, imag as Y for standard complex plane plot
    return PlotData(real_parts, imag_parts, config);
}

std::string_view PlotEngine::PolarPlot(std::string_view r_expression, const PlotConfig& config) noexcept {
    arena_.reset();
    AlgebraicParser parser;
    const int width = std::clamp(config.width, 1, 256);
    const int height = std::clamp(config.height, 1, 256);
    
    char grid[65536];
    std::memset(grid, ' ', static_cast<size_t>(width) * height);
    
    const int samples = 720;
    const double theta_step = 2.0 * M_PI / samples;
    
    for (int i = 0; i < samples; ++i) {
        const double theta = i * theta_step;
        SymbolTable context;
        context.push_back({"theta", Number(theta)});
        context.push_back({"t", Number(theta)});
        
        const auto res = parser.ParseAndExecuteWithContext(r_expression, context);
        if (const auto r_opt = res.GetDouble(); r_opt) {
            const double r = *r_opt;
            const double x = r * std::cos(theta);
            const double y = r * std::sin(theta);
            
            if (std::isfinite(x) && std::isfinite(y) &&
                x >= config.x_min && x <= config.x_max &&
                y >= config.y_min && y <= config.y_max) {
                const auto [screen_x, screen_y] = MapToScreen(x, y, config);
                if (screen_x >= 0 && screen_x < width && screen_y >= 0 && screen_y < height) {
                    grid[screen_y * width + screen_x] = config.plot_char;
                }
            }
        }
    }
    
    if (config.show_axes) {
        const auto [ax, ay] = MapToScreen(0, 0, config);
        if (ax >= 0 && ax < width) for (int r = 0; r < height; ++r) if (grid[r * width + ax] == ' ') grid[r * width + ax] = '|';
        if (ay >= 0 && ay < height) for (int c = 0; c < width; ++c) if (grid[ay * width + c] == ' ') grid[ay * width + c] = '-';
    }

    FixedVector<char, 70000> result_buffer;
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) result_buffer.push_back(grid[row * width + col]);
        result_buffer.push_back('\n');
    }
    return arena_.allocString(std::string_view(result_buffer.data(), result_buffer.size()));
}

} // namespace AXIOM
