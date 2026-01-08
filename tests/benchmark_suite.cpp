// AXIOM Engine v3.0 - Performance Benchmark Suite
// Measures execution time and throughput for critical operations

#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <numeric>
#include <algorithm>

#include "../include/algebraic_parser.h"
#include "../include/statistics_engine.h"
#include "../include/linear_system_parser.h"
#include "../include/eigen_engine.h"
#include "../include/symbolic_engine.h"

using namespace std::chrono;

class BenchmarkRunner {
public:
    struct BenchmarkResult {
        std::string name;
        double avg_time_ms;
        double min_time_ms;
        double max_time_ms;
        double ops_per_second;
        size_t iterations;
    };

    template<typename Func>
    BenchmarkResult RunBenchmark(const std::string& name, Func func, size_t iterations = 1000) {
        std::vector<double> times;
        times.reserve(iterations);

        // Warmup
        for (size_t i = 0; i < 10; ++i) {
            func();
        }

        // Actual benchmark
        for (size_t i = 0; i < iterations; ++i) {
            auto start = high_resolution_clock::now();
            func();
            auto end = high_resolution_clock::now();
            
            duration<double, std::milli> elapsed = end - start;
            times.push_back(elapsed.count());
        }

        double sum = std::accumulate(times.begin(), times.end(), 0.0);
        double avg = sum / times.size();
        double min = *std::min_element(times.begin(), times.end());
        double max = *std::max_element(times.begin(), times.end());
        double ops_per_sec = 1000.0 / avg;

        return {name, avg, min, max, ops_per_sec, iterations};
    }

    void PrintResult(const BenchmarkResult& result) {
        std::cout << "  [BENCH] " << std::left << std::setw(50) << result.name;
        std::cout << std::right << std::setw(10) << std::fixed << std::setprecision(4) << result.avg_time_ms << " ms";
        std::cout << std::setw(12) << std::setprecision(0) << result.ops_per_second << " ops/s";
        std::cout << " (min: " << std::fixed << std::setprecision(4) << result.min_time_ms << " ms, ";
        std::cout << "max: " << result.max_time_ms << " ms)" << std::endl;
    }

    void PrintHeader(const std::string& section) {
        std::cout << "\n========================================" << std::endl;
        std::cout << "  " << section << std::endl;
        std::cout << "========================================" << std::endl;
    }
};

void BenchmarkAlgebraicParser() {
    BenchmarkRunner runner;
    runner.PrintHeader("ALGEBRAIC PARSER BENCHMARKS");

    AXIOM::AlgebraicParser parser;

    // Simple arithmetic
    auto result1 = runner.RunBenchmark("Simple addition (5+5)", [&]() {
        parser.Parse("5+5");
    });
    runner.PrintResult(result1);

    auto result2 = runner.RunBenchmark("Complex expression (2*x + 3*y - sin(30))", [&]() {
        std::map<std::string, double> vars = {{"x", 10.0}, {"y", 20.0}};
        parser.Parse("2*x + 3*y - sin(30)", vars);
    });
    runner.PrintResult(result2);

    auto result3 = runner.RunBenchmark("Nested parentheses ((2+3)*(4+5))", [&]() {
        parser.Parse("((2+3)*(4+5))");
    });
    runner.PrintResult(result3);

    auto result4 = runner.RunBenchmark("Trigonometric functions (sin(x)*cos(y))", [&]() {
        std::map<std::string, double> vars = {{"x", 45.0}, {"y", 60.0}};
        parser.Parse("sin(x)*cos(y)", vars);
    });
    runner.PrintResult(result4);

    auto result5 = runner.RunBenchmark("Power and root (2^10 + sqrt(144))", [&]() {
        parser.Parse("2^10 + sqrt(144)");
    });
    runner.PrintResult(result5);
}

void BenchmarkStatisticsEngine() {
    BenchmarkRunner runner;
    runner.PrintHeader("STATISTICS ENGINE BENCHMARKS");

    AXIOM::StatisticsEngine stats;

    // Small dataset
    std::vector<double> small_data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};

    auto result1 = runner.RunBenchmark("Mean (n=10)", [&]() {
        stats.Mean(small_data);
    });
    runner.PrintResult(result1);

    auto result2 = runner.RunBenchmark("Variance (n=10)", [&]() {
        stats.Variance(small_data);
    });
    runner.PrintResult(result2);

    auto result3 = runner.RunBenchmark("StandardDeviation (n=10)", [&]() {
        stats.StandardDeviation(small_data);
    });
    runner.PrintResult(result3);

    // Medium dataset
    std::vector<double> medium_data(1000);
    std::iota(medium_data.begin(), medium_data.end(), 1.0);

    auto result4 = runner.RunBenchmark("Mean (n=1000)", [&]() {
        stats.Mean(medium_data);
    }, 500);
    runner.PrintResult(result4);

    auto result5 = runner.RunBenchmark("Variance (n=1000)", [&]() {
        stats.Variance(medium_data);
    }, 500);
    runner.PrintResult(result5);

    auto result6 = runner.RunBenchmark("Median (n=1000)", [&]() {
        stats.Median(medium_data);
    }, 500);
    runner.PrintResult(result6);

    // Large dataset
    std::vector<double> large_data(10000);
    std::iota(large_data.begin(), large_data.end(), 1.0);

    auto result7 = runner.RunBenchmark("Mean (n=10000)", [&]() {
        stats.Mean(large_data);
    }, 100);
    runner.PrintResult(result7);

    auto result8 = runner.RunBenchmark("StandardDeviation (n=10000)", [&]() {
        stats.StandardDeviation(large_data);
    }, 100);
    runner.PrintResult(result8);

    // Correlation
    std::vector<double> x_data(100), y_data(100);
    std::iota(x_data.begin(), x_data.end(), 1.0);
    std::iota(y_data.begin(), y_data.end(), 2.0);

    auto result9 = runner.RunBenchmark("Correlation (n=100)", [&]() {
        stats.Correlation(x_data, y_data);
    }, 500);
    runner.PrintResult(result9);

    auto result10 = runner.RunBenchmark("LinearRegression (n=100)", [&]() {
        stats.LinearRegression(x_data, y_data);
    }, 500);
    runner.PrintResult(result10);
}

void BenchmarkLinearSystemParser() {
    BenchmarkRunner runner;
    runner.PrintHeader("LINEAR SYSTEM PARSER BENCHMARKS");

    LinearSystemParser parser;

    // 2x2 system
    auto result1 = runner.RunBenchmark("Solve 2x2 system (matrix notation)", [&]() {
        parser.ParseAndExecute("solve [[2,1],[1,3]] [8,13]");
    });
    runner.PrintResult(result1);

    // 3x3 system
    auto result2 = runner.RunBenchmark("Solve 3x3 system (matrix notation)", [&]() {
        parser.ParseAndExecute("solve [[2,1,1],[1,3,2],[1,0,0]] [4,5,6]");
    });
    runner.PrintResult(result2);

    // 5x5 system
    auto result3 = runner.RunBenchmark("Solve 5x5 system", [&]() {
        parser.ParseAndExecute("solve [[5,2,1,0,0],[2,6,3,1,0],[1,3,7,4,2],[0,1,4,8,5],[0,0,2,5,9]] [1,2,3,4,5]");
    }, 500);
    runner.PrintResult(result3);

    // Cramer's rule
    auto result4 = runner.RunBenchmark("Cramer's rule 2x2", [&]() {
        parser.ParseAndExecute("cramer 2x+3y=8; 4x+5y=14");
    });
    runner.PrintResult(result4);
}

#ifdef ENABLE_EIGEN
void BenchmarkEigenEngine() {
    BenchmarkRunner runner;
    runner.PrintHeader("EIGEN ENGINE BENCHMARKS");

    AXIOM::EigenEngine eigen;

    // Small matrices
    std::vector<std::vector<double>> small_a = {{1, 2}, {3, 4}};
    std::vector<std::vector<double>> small_b = {{5, 6}, {7, 8}};

    auto result1 = runner.RunBenchmark("Matrix multiply 2x2", [&]() {
        auto A = eigen.CreateMatrix(small_a);
        auto B = eigen.CreateMatrix(small_b);
        eigen.MatrixMultiply(A, B);
    });
    runner.PrintResult(result1);

    auto result2 = runner.RunBenchmark("Matrix inverse 2x2", [&]() {
        auto A = eigen.CreateMatrix(small_a);
        eigen.Inverse(A);
    });
    runner.PrintResult(result2);

    auto result3 = runner.RunBenchmark("Determinant 2x2", [&]() {
        auto A = eigen.CreateMatrix(small_a);
        eigen.Determinant(A);
    });
    runner.PrintResult(result3);

    // Medium matrices
    std::vector<std::vector<double>> med_a(10, std::vector<double>(10));
    std::vector<std::vector<double>> med_b(10, std::vector<double>(10));
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            med_a[i][j] = i * 10 + j + 1;
            med_b[i][j] = (i + j) * 0.5;
        }
    }

    auto result4 = runner.RunBenchmark("Matrix multiply 10x10", [&]() {
        auto A = eigen.CreateMatrix(med_a);
        auto B = eigen.CreateMatrix(med_b);
        eigen.MatrixMultiply(A, B);
    }, 500);
    runner.PrintResult(result4);

    auto result5 = runner.RunBenchmark("Determinant 10x10", [&]() {
        auto A = eigen.CreateMatrix(med_a);
        eigen.Determinant(A);
    }, 500);
    runner.PrintResult(result5);

    // Linear system
    std::vector<std::vector<double>> sys_a = {{2, 1}, {1, 3}};
    std::vector<double> sys_b = {8, 13};

    auto result6 = runner.RunBenchmark("Solve linear system 2x2", [&]() {
        auto A = eigen.CreateMatrix(sys_a);
        auto b = eigen.CreateVector(sys_b);
        eigen.SolveLinearSystem(A, b);
    });
    runner.PrintResult(result6);

    // Large matrices
    std::vector<std::vector<double>> large_a(100, std::vector<double>(100));
    std::vector<std::vector<double>> large_b(100, std::vector<double>(100));
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; ++j) {
            large_a[i][j] = (i * 100 + j + 1) * 0.01;
            large_b[i][j] = ((i + j) * 0.5) * 0.01;
        }
    }

    auto result7 = runner.RunBenchmark("Matrix multiply 100x100", [&]() {
        auto A = eigen.CreateMatrix(large_a);
        auto B = eigen.CreateMatrix(large_b);
        eigen.MatrixMultiply(A, B);
    }, 50);
    runner.PrintResult(result7);

    auto result8 = runner.RunBenchmark("Determinant 100x100", [&]() {
        auto A = eigen.CreateMatrix(large_a);
        eigen.Determinant(A);
    }, 50);
    runner.PrintResult(result8);
}
#endif

void BenchmarkSymbolicEngine() {
    BenchmarkRunner runner;
    runner.PrintHeader("SYMBOLIC ENGINE BENCHMARKS");

    AXIOM::SymbolicEngine symbolic;

    auto result1 = runner.RunBenchmark("Expand (x+1)^2", [&]() {
        symbolic.Expand("(x+1)^2");
    });
    runner.PrintResult(result1);

    auto result2 = runner.RunBenchmark("Simplify (x^2 + 2*x + 1)", [&]() {
        symbolic.Simplify("x^2 + 2*x + 1");
    });
    runner.PrintResult(result2);

    auto result3 = runner.RunBenchmark("Integrate x^2", [&]() {
        symbolic.Integrate("x^2", "x");
    });
    runner.PrintResult(result3);

    auto result4 = runner.RunBenchmark("Differentiate x^3", [&]() {
        symbolic.Differentiate("x^3", "x");
    });
    runner.PrintResult(result4);

    auto result5 = runner.RunBenchmark("Factor x^2 - 1", [&]() {
        symbolic.Factor("x^2 - 1");
    });
    runner.PrintResult(result5);

    auto result6 = runner.RunBenchmark("Substitute x=5 in x+3", [&]() {
        symbolic.Substitute("x+3", "x", "5");
    });
    runner.PrintResult(result6);

    auto result7 = runner.RunBenchmark("Expand (x+y)^3", [&]() {
        symbolic.Expand("(x+y)^3");
    });
    runner.PrintResult(result7);

    auto result8 = runner.RunBenchmark("Integrate sin(x)", [&]() {
        symbolic.Integrate("sin(x)", "x");
    });
    runner.PrintResult(result8);
}

int main() {
    std::cout << "\n╔═══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                                                               ║" << std::endl;
    std::cout << "║        AXIOM ENGINE v3.0 - PERFORMANCE BENCHMARKS             ║" << std::endl;
    std::cout << "║                                                               ║" << std::endl;
    std::cout << "║          Measuring Throughput & Execution Time                ║" << std::endl;
    std::cout << "║                                                               ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════════════╝" << std::endl;

    auto total_start = high_resolution_clock::now();

    BenchmarkAlgebraicParser();
    BenchmarkStatisticsEngine();
    BenchmarkLinearSystemParser();
#ifdef ENABLE_EIGEN
    BenchmarkEigenEngine();
#endif
    BenchmarkSymbolicEngine();

    auto total_end = high_resolution_clock::now();
    duration<double, std::milli> total_elapsed = total_end - total_start;

    std::cout << "\n========================================" << std::endl;
    std::cout << "  BENCHMARK SUMMARY" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Total benchmark time: " << std::fixed << std::setprecision(2) 
              << total_elapsed.count() << " ms" << std::endl;
    std::cout << "\n✓ ALL BENCHMARKS COMPLETED!" << std::endl;

    return 0;
}
