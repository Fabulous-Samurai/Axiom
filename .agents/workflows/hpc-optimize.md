---
description: How to apply Zenith HPC/HFT performance optimizations to the AXIOM Engine
---

# HPC/HFT Performance Optimization (Zenith Edition)

## Zenith Pillars
- **Pillar 1 (Zero-Allocation)**: Use `HarmonicArena` for all burst telemetry and ingress data.
- **Pillar 3 (Determinizm)**: Measure everything in RDTSC cycles. Goal: <0.255ns dispatch jitter.
- **Pillar 5 (Zero-Exception)**: No CPU branches for exception handling.

## Performance Goals (Zenith V2.0)
| Module | Target | Mechanism |
|--------|--------|-----------|
| **Evaluation** | <0.5ns | Static Variant Dispatch (`std::visit`) |
| **Telemetry** | <1.0ns | Lock-free SPSC Ring Buffer |
| **Ingress** | <50ns | AF_XDP Kernel-Bypass DMA |
| **ECU Inference** | Deterministic | `VirtualLock` / `mlock` weight pinning |

## Key Optimization Files
- `include/cpu_optimization.h`: `AXIOM_RDTSC`, `AXIOM_FORCE_INLINE`.
- `src/telemetry.cpp`: Nanosaniye-precision zero-observer logging.
- `src/cognitive_ecu.cpp`: Pinned memory and tensor pools.
- `src/ingress.cpp`: Zero-copy network ingress patterns.

## Performance Validation (RDTSC)

1. **Cycle Baseline**
   ```bash
   build/axiom_benchmark.exe --benchmark_filter=RDTSC_Overhead
   ```

2. **Dispatcher Jitter Test**
   ```bash
   build/axiom_benchmark.exe --benchmark_filter=Dispatch_Latency
   ```

3. **Telemetry Pressure Test**
   ```bash
   build/axiom_benchmark.exe --benchmark_filter=Telemetry_Contention
   ```

## Optimization Rules
- **Rule 1**: If it's not inlined, it's too slow.
- **Rule 2**: If it calls a mutex, it's not Zenith.
- **Rule 3**: If it allocates on the heap during `Evaluate`, it's a Pillar violation.

## Validation
- **Heisenberg Check**: Compare evaluation latency with telemetry enabled vs disabled. Overhead must be <2 RDTSC cycles.
- **Memory Pinning Verification**: `CognitiveECU` must report "Weights pinned successfully".

## Rollback
- Revert performance-degrading changes and re-verify cycle counts.
