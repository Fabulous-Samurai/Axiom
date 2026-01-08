# Project Reorganization Plan

## Current Structure (CHAOTIC) ❌
```
root/
  ├── comprehensive_test_suite.py
  ├── quick_perf_test.py
  ├── test_log2.py
  ├── test_direct.py
  ├── test_log_functions.py
  ├── test_compile.cpp
  ├── build_and_test.bat
  ├── run_all_tests.bat
  ├── COMPLEX_PLOT_TESTS.py
  ├── OPTIMIZATION_REPORT_DEC25.md
  ├── ERROR_FIXES_SUMMARY.md
  ├── tests/ (unit + integration)
  ├── docs/
  ├── scripts/
  └── ... (many more scattered files)
```

## New Structure (CLEAN) ✅
```
root/
  ├── src/                      # C++ source files
  ├── include/                  # C++ headers
  ├── tests/                    # ALL TESTS HERE
  │   ├── unit/                 # Unit tests (existing)
  │   ├── integration/          # Integration tests (existing)
  │   ├── performance/          # Performance tests (NEW)
  │   │   ├── comprehensive_test_suite.py
  │   │   ├── quick_perf_test.py
  │   │   └── README.md
  │   ├── functional/           # Functional tests (NEW)
  │   │   ├── test_log2.py
  │   │   ├── test_direct.py
  │   │   ├── test_log_functions.py
  │   │   └── test_persistent.py
  │   ├── examples/             # Test examples (NEW)
  │   │   └── COMPLEX_PLOT_TESTS.py
  │   └── README.md
  ├── scripts/                  # Build & utility scripts
  │   ├── build/                # Build scripts (NEW)
  │   │   ├── fast_build.ps1
  │   │   ├── ninja_build.bat
  │   │   └── ninja_build.sh
  │   ├── test/                 # Test runner scripts (NEW)
  │   │   ├── run_all_tests.bat
  │   │   └── build_and_test.bat
  │   └── README.md
  ├── docs/                     # Documentation
  │   ├── reports/              # Analysis reports (NEW)
  │   │   ├── OPTIMIZATION_REPORT_DEC25.md
  │   │   ├── ERROR_FIXES_SUMMARY.md
  │   │   ├── PERFORMANCE_SLOWDOWN_ANALYSIS.md
  │   │   └── OPTIMIZATION_REPORT.md
  │   ├── api/
  │   ├── qa/
  │   └── user/
  ├── build/                    # Build artifacts
  ├── ninja-build/              # Ninja build directory
  ├── gui/                      # GUI application
  ├── examples/                 # Usage examples
  ├── tools/                    # Development tools
  └── core/                     # Core engine modules
```
