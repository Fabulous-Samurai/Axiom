---
description: How to benchmark Phase F Cognitive ECU & SLM pinning
---

# Benchmark Cognitive ECU

## Zenith Pillars
- **Pillar 1 (Zero-Allocation)**: Verifies NO heap churn during SLM inference.
- **Pillar 3 (Determinizm)**: Verifies that memory pinning eliminates page-fault jitter.

## Purpose
Validate the performance and determinism of the Cognitive ECU SLM isolation.

## Steps

1. **Run Pinning Diagnostic**
   ```bash
   build/run_tests.exe --gtest_filter=CognitiveECU_Pinning
   ```
   *Expected: [AXIOM Phase F] Weights pinned successfully*

2. **Inference Latency Stress**
   ```bash
   build/axiom_benchmark.exe --benchmark_filter=ECU_Inference
   ```

3. **Memory Pressure Baseline**
   Force system memory pressure and verify that the pinned pool does NOT swap.

## Validation
- "Weights pinned successfully" appears in standard output.
- Inference latency variance (jitter) is < 1%.
- Zero `malloc` calls detected by the memory profiler during `run_inference`.
