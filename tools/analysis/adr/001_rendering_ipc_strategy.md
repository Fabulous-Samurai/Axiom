# ADR-001: Symbiotic Hybrid Rendering & Zero-Copy IPC

## Status
Accepted (2026-03-24)

## Context
AXIOM Studio requires visualizing 2.5M ops/sec with <16ms frame latency ("Dirac" aesthetic). Traditional IPC (JSON/Sockets) and high-level UI frameworks (Electron/Standard QML) introduce unacceptable overhead.

## Decision
1. **Rendering:** Adopt a hybrid approach using **Qt RHI (QRhi)** for standard UI elements and **Native Vulkan Injection** via `QSGRenderNode` for high-frequency HPC data visualization.
2. **Data Plane:** Implement **Zero-Copy Shared Memory (Windows Section Objects)** with a ring-buffer protocol between `axiom_engine` and `axiom_studio`.
3. **Shader Strategy:** Use precompiled SPIR-V shaders with **Specialization Constants** for real-time performance tuning.

## Consequences
- **Pros:** Near-zero CPU overhead for data transfer, cross-platform UI stability, and maximum GPU utilization for complex visualizations.
- **Cons:** Increased implementation complexity in the bridge layer (C++), OS-specific memory mapping logic.

## Technical Trick
Inject native Vulkan command buffers directly into Qt's render pass using `nativeHandles()`, allowing high-performance overlays on top of a stable QML frame.
