---
description: How to build and verify the AXIOM Zenith Engine
---

# Build AXIOM Zenith Engine

## Zenith Pillars
- **Pillar 1 (Zero-Allocation)**: Validated via `run_tests.exe` memory leak checks.
- **Pillar 5 (Zero-Exception)**: Validated via `noexcept` build-time checks and tests.

## Purpose
Build and verify AXIOM Engine binaries, including the new Zenith expansion modules (E-F-H).

## Preconditions
- CMake >= 3.20 (Required for C++23)
- Ninja Build System
- Windows: MSVC v143+ (Visual Studio 2022)
- Linux: GCC 13+ or Clang 16+

## Inputs
- Source tree (repo root)
- `CMakeLists.txt`, `CMakePresets.json`

## Outputs
- `build/axiom.exe` (Main Engine with E-F-H)
- `build/run_tests.exe` (Unit Test Suite)
- `build/axiom_benchmark.exe` (RDTSC Performance Suite)

## Steps

1. **Configure Environment**
   ```bash
   cmake --preset default-ninja
   ```

2. **Full Rebuild (Mandatory Path)**
   ```bash
   cmake --build build --config Release --parallel
   ```

3. **Verify Core Reliability**
   ```bash
   build/run_tests.exe
   ```

4. **Initialize Telemetry and Verify**
   ```bash
   build/axiom.exe "2+2"
   # Ensure axiom_telemetry.log is created
   ```

## Validation
- Build exits with code 0.
- All unit tests pass.
- `axiom_telemetry.log` contains valid RDTSC markers (0x1000, 0x1001).

## Rollback
- Revert suspected commits and perform a clean build: `cmake --build build --target clean`.
