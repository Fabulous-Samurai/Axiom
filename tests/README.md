# AXIOM Engine Tests

Comprehensive test suite for the AXIOM computational engine.

## Directory Structure

```
tests/
├── unit/              # Unit tests for individual components
├── integration/       # Integration tests for module interactions
├── performance/       # Performance benchmarking tests
├── functional/        # Functional end-to-end tests
└── examples/          # Example test cases and demonstrations
```

## Running Tests

### All Tests
```bash
# Windows
scripts\test\run_all_tests.bat

# Linux/Mac
scripts/test/run_all_tests.sh
```

### Performance Tests Only
```bash
cd tests/performance
python comprehensive_test_suite.py  # Full test suite (41 tests)
python quick_perf_test.py           # Quick performance check (10 tests)
```

### Functional Tests Only
```bash
cd tests/functional
python test_log2.py                 # Test log2 function
python test_persistent.py           # Test persistent subprocess
```

### Unit Tests
```bash
cd tests/unit
# Run with your C++ test framework
```

## Test Categories

### Performance Tests (`performance/`)
- **comprehensive_test_suite.py**: Complete test suite covering all mathematical operations
  - 41 tests across 7 categories
  - Basic arithmetic, functions, compositions, special ops
  - Performance metrics and speed ratings
  
- **quick_perf_test.py**: Fast performance verification
  - 10 critical tests
  - Sub-millisecond execution validation
  - Persistent subprocess verification

### Functional Tests (`functional/`)
- **test_log2.py**: Logarithm base-2 function tests
- **test_direct.py**: Direct C++ engine communication tests
- **test_log_functions.py**: All logarithm function tests
- **test_persistent.py**: Persistent subprocess mode tests

### Example Tests (`examples/`)
- **COMPLEX_PLOT_TESTS.py**: 61 advanced plotting test cases
  - Multi-function overlays
  - Statistical distributions
  - Parametric curves
  - Quantum mechanics visualizations
  - Performance stress tests

## Performance Baselines

### Expected Results (Persistent Subprocess Mode)
- **Average Response Time**: <1ms
- **Throughput**: 1000+ commands/second
- **Success Rate**: 97-100%
- **Speed Rating**: ⚡ SENNA (hyper-optimized)

### Performance Categories
- ⚡ SENNA: <5ms (hyper-optimized)
- 🏎️ F1: 5-20ms (very fast)
- 🚗 NORMAL: 20-50ms (acceptable)
- 🐌 SLOW: >50ms (needs optimization)

## Test Results

Test results are automatically saved to:
- `test_results_comprehensive.txt`
- `test_results_performance.txt`
- `test_results_functional.txt`

## Adding New Tests

### Performance Test
1. Add test case to `performance/comprehensive_test_suite.py`
2. Follow existing category structure
3. Include expected performance baseline

### Functional Test
1. Create new file in `functional/`
2. Import `gui_helpers.CppEngineInterface`
3. Test specific functionality
4. Add to test runner script

### Example Test
1. Add to `examples/COMPLEX_PLOT_TESTS.py`
2. Include performance notes
3. Document any known limitations

## Continuous Integration

Tests are run on:
- Every commit (unit + integration)
- Pull requests (full suite)
- Nightly (performance benchmarks)

## Troubleshooting

### Common Issues

**Test Hangs/Timeout:**
- Complex expressions may take longer (see PERFORMANCE_SLOWDOWN_ANALYSIS.md)
- Simplify expressions or increase timeout

**Import Errors:**
- Ensure Python path includes `gui/python`
- Check `sys.path.insert()` in test files

**Build Required:**
- Run `scripts/build/fast_build.ps1` first
- Ensure `ninja-build/axiom.exe` exists

## Documentation

- **Performance Analysis**: [docs/reports/PERFORMANCE_SLOWDOWN_ANALYSIS.md](../docs/reports/PERFORMANCE_SLOWDOWN_ANALYSIS.md)
- **Optimization Reports**: [docs/reports/](../docs/reports/)
- **QA Reports**: [docs/qa/](../docs/qa/)
