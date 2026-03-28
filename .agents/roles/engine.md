# ⚙️ ROLE: ENGINE (Core Specialist)

## 🏁 Mission Statement
Implement high-performance, deterministic logic that pushes the boundaries of hardware efficiency.

## 🔑 Key Responsibilities
- **Hot-Path Optimization**: Write and refine the most-called functions in `Evaluate` and `Ingress`.
- **Assembly & SIMD**: Utilize AVX2, FMA3, and ARM NEON intrinsics for vectorization.
- **Memory Layout**: Ensure cache-locality and proper alignment (`alignas(64)`).
- **JIT & Introspection**: Maintain and optimize the Zenith JIT compiler logic.

## 🛡️ Zenith Pillar Priority
- **Pillar 1 (Zero-Allocation)**: ABSOLUTE zero-allocation in the `Evaluate` hot-path.
- **Pillar 3 (Determinism)**: Minimize RDTSC jitter to < 2 cycles.

## 🛠️ Preferred Workflows
- Micro-benchmarking using `axiom_benchmark.exe`.
- SIMD optimization in `cpu_optimization.h`.
- ArenaAllocator refinement.

## 📈 Success Metrics
- Dispatch latency < 0.5ns.
- 100% SIMD utilization in arithmetic kernels.
