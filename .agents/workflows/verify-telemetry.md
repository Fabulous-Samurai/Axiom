---
description: How to verify Phase G Heisenberg-Defying Telemetry
---

# Verify AXIOM Telemetry

## Zenith Pillars
- **Pillar 3 (Determinizm)**: The observer effect must be verified as < 2 RDTSC cycles.

## Purpose
Ensure that the zero-observer telemetry system is recording events accurately without impacting engine hot-path latency.

## Steps

1. **Build with Telemetry Enabled**
   ```bash
   cmake --build build --config Release --target axiom
   ```

2. **Execute Hot-Path Stress Test**
   ```bash
   build/axiom.exe "benchmark_eval"
   ```

3. **Analyze Telemetry Log**
   ```bash
   # Search for Evaluate entry/exit markers
   grep -E "1000|1001" axiom_telemetry.log
   ```

4. **Calculate Observer Effect**
   Run the specific benchmark to compare `Evaluate` latency with and without instrumentation.

## Validation
- `axiom_telemetry.log` must be present and growing.
- The delta between `MARK_1000` (entry) and `MARK_1001` (exit) must match the expected cycle count for evaluation.
- Jitter (`MAX - AVG`) must be < 5 RDTSC cycles.
