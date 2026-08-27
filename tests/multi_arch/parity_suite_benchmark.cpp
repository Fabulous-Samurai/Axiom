#include <benchmark/benchmark.h>

#include <cmath>
#include <vector>

#include "algebraic_parser.h"
#include "zenith_jit.h"

using namespace AXIOM;

// Generic Math benchmark to measure throughput differences
static void BM_MathThroughput(benchmark::State& state) {
  AlgebraicParser parser;
  ZenithJIT jit;

  // Complex mathematical expression for high-throughput calculation
  std::string expr = "sin(x) * cos(y) + tan(x) / (x * x + y * y + 1.0)";
  NodePtr root = parser.ParseExpression(expr);
  SymbolTable var_map;
  var_map.push_back({"x", 0.0});
  var_map.push_back({"y", 0.0});
  JiffedFunc fn = jit.Compile(root, var_map);

  double vars[2] = {0.5, 1.2};
  for (auto _ : state) {
    vars[0] += 0.001;
    vars[1] -= 0.001;
    double result = fn(vars);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_MathThroughput);

// Matrix SIMD Math Benchmark
static void BM_MatrixSIMDMath(benchmark::State& state) {
  AlgebraicParser parser;
  ZenithJIT jit;

  std::string expr = "sin(M) + M * 2.0";
  NodePtr root = parser.ParseExpression(expr);
  SymbolTable var_map;
  var_map.push_back({"M", 0.0});
  JiffedMatrixFunc fn = jit.CompileMatrix(root, var_map);

  double flat_matrix[16] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8,
                            0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6};
  double results[16] = {0};

  for (auto _ : state) {
    fn(flat_matrix, results);
    benchmark::DoNotOptimize(results);
  }
}
BENCHMARK(BM_MatrixSIMDMath);

// Architecture-specific macro definition for reporting
static void BM_ArchitectureReporting(benchmark::State& state) {
  for (auto _ : state) {
#if defined(__x86_64__) || defined(_M_X64)
    benchmark::DoNotOptimize("x86_64");
#elif defined(__aarch64__) || defined(_M_ARM64)
    benchmark::DoNotOptimize("ARM64");
#else
    benchmark::DoNotOptimize("Unknown");
#endif
  }
}
BENCHMARK(BM_ArchitectureReporting);

int main(int argc, char** argv) {
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  ::benchmark::RunSpecifiedBenchmarks();
  ::benchmark::Shutdown();
  return 0;
}
