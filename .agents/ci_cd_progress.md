# AXIOM CI/CD Integration Progress

## 📝 Overview
This document tracks the journey of integrating and stabilizing the AXIOM Engine CI/CD pipeline on GitHub Actions.

## ✅ Completed Actions

### 1. Workflow Consolidation
- Unified 4 disparate workflows (`ci.yml`, `axiom_zenith_ci.yml`, `enforce_no_dynamic_allocs.yml`, `valgrind-analysis.yml`) into a single, modular `axiom_zenith_ci.yml`.
- Later renamed/refactored to **AXIOM Zenith Modular CI** to allow isolated Phase 0 (Hygiene/Formal), Phase 1 (Build), and Phase 2 (Test).

### 2. Core Build & SIMD Fixes
- Fixed AVX2/FMA flag placement in `CMakeLists.txt` which was causing compilation failures in `linear_system_parser.cpp`.
- Improved architecture detection in `scripts/setup_other_device_unix.sh` to use conservative flags (`x86-64`, `armv8-a`) in CI environments.
- Disabled LTO (Link Time Optimization) for Debug builds to improve stability and build speed.

### 3. Zenith Pillar Compliance
- Identified 192 violations in the initial scan.
- Refactored `PlutoController` to use `AXIOM::FixedVector` instead of `std::vector` to satisfy the **Zero-Allocation** mandate.
- Replaced `resize()` calls with static capacity (`Mantis::kMaxNodes`).

### 4. Formal Verification (TLA+)
- Fixed TLA+ step in CI to use the existing `PlutoPetriNet.tla` model.
- Integrated `scripts/formal/tla/run_pluto_verification.py` into the workflow.
- Handled Java classpaths and working directory issues for `tla2tools.jar`.

### 5. Dependency & Linking Fixes
- Added missing `AXIOM_EXPORT` to `PlutoController` and `IDAStarSolver` for Windows DLL compatibility.
- Linked new tests (`test_ida_star`, `test_pluto_core`) against `axiom_engine`, `Threads`, `asmjit`, and `Python`.
- Fixed "out of scope" errors by adding missing headers:
    - `<utility>` in `fixed_vector.h` (for `std::move`).
    - `<cmath>` and `<limits>` in `mantis_solver.h` (for `std::pow`, `std::numeric_limits`).

### 6. Missing Files Discovery
- Added `tests/unit/test_ida_star.cpp` which was referenced in `CMakeLists.txt` but missing from the repository.

### 7. Modular CI/CD (Current Architecture)
- **Phase 0:** Pre-Commit Linting (continue-on-error) & TLA+ Verification.
- **Phase 1:** Parallel Builds for Linux and Windows.
- **Phase 2:** Isolated Test Execution with detailed logging (`LastTest.log` output on failure).
- **Phase 3:** SonarCloud Quality Gate.

## 🚧 In Progress / Current Status
- Monitoring Modular CI run `671a0a4`.
- **Phase 0 (Hygiene/Formal):** Success.
- **Phase 1 (Build Linux):** Persistent Failure. `ninja -v` and `DCMAKE_VERBOSE_MAKEFILE=ON` enabled to catch the error.
- **Phase 1 (Build Windows):** Success.
- **Phase 2 (Test Windows):** In Progress. Timeout (300s) and parallelism (-j4) added.
- **Phase 2 (Test Linux):** Skipped due to build failure.
- **Investigating:** SIMD alignment, Python headers, and template linkage on Linux.

## ❌ Reverted / Attempted
- **mimalloc Integration:** Attempted to link `axiom_engine` explicitly to `mimalloc` with `MI_OVERRIDE`. Reverted due to widespread build failures across all platforms. Likely requires more careful configuration or platform-specific guards.

---
*Last Updated: 2026-03-22*
