---
title: Global Agent Process Protocol
version: 2.0
author: Principal Systems & Performance Architect / Fabulous-Samurai
---

> **Status Update**: The repository has transitioned to **AXIOM Zenith (Phases E-H)**. All agents must now adhere to the Zenith Master Architect V2.0 protocols, prioritizing zero-observer telemetry, kernel-bypass networking, and hardware-secured enclaves.

# Global Agent Process Protocol (Zenith Edition)

## Purpose

This document defines the strict operational standards for autonomous agents, performance architects, and CI/CD workflows within the AXIOM Zenith ecosystem.

## Pillars of Zenith (Mandatory)

1. **Pillar 1: Zero-Allocation**: No heap allocations (`new`, `malloc`, `std::vector::push_back` beyond capacity) are permitted in the `Evaluate` or `Ingress` hot-paths. Use `ArenaAllocator` or `HarmonicArena`.
2. **Pillar 3: Determinizm (RDTSC)**: Performance must be measured in CPU cycles using `AXIOM_RDTSC`. Any optimization that introduces non-deterministic jitter (e.g., OS rescheduling, swap) must be mitigated via CPU pinning or `mlock`.
3. **Pillar 5: Zero-Exception**: Critical engine paths must be `noexcept`. Use `std::optional`, `std::expected` (C++23), or error codes.

---

### Basic Principles

- **Security**: Hardware enclaves (Phase H) protect the Mantis state machine. Agents must never log or leak raw state memory from secured regions.
- **Repeatability**: Benchmarks must be run with fixed CPU frequency and isolated cores to ensure RDTSC consistency.
- **Zero-Observer Telemetry**: Use only the `TelemetryScribe` (lock-free SPSC) for logging during performance critical sections. `std::cout` or `printf` are strictly forbidden in hot-paths.

---

### Zenith Rules

1. **Kernel-Bypass (Phase E)**: Standard POSIX `recv`/`send` are deprecated for high-velocity feeds. Agents must utilize `AF_XDP` or `IngressChannel` DMA paths.
2. **Cognitive ECU (Phase F)**: Model weights must be pinned using `PinnedMemoryPool` (`VirtualLock`/`mlock`). Inference must occur in pre-allocated tensor buffers.
3. **Heisenberg-Defying Telemetry (Phase G)**: All latency measurements must use the internal `TelemetryScribe`. External profilers that introduce >10ns overhead are disallowed for micro-benchmarking.
4. **Hardware Security (Phase H)**: State machine transitions must be modeled as Enclave ECALLs. Secure memory must be cleared immediately after unsealing.
5. **C++23 Zenith Standard**: Leverage `std::from_chars`, `std::format`, and `std::expected` to maintain zero-exception and high-performance parsing.

---

### Workflow Template (Zenith Required)

- **metadata** (description, author, version)
- **Zenith Pillars** (Which pillars does this workflow enforce?)
- **Performance Impact** (Expected impact on RDTSC cycles)
- **Steps** (Deterministic, numbered steps)
- **Validation** (Using `AXIOM_TELEMETRY_MARK` or cycle-counts)

---

### Versioning

- Version increases are mandatory for any change affecting the allocator, dispatcher, or security enclaves.
- Current Version: 2.0 (Zenith Architect)

---

### Communication

- For Zenith-level architectural deviations, contact: `Fabulous-Samurai`.
- Rule: **No Hedging**. The Mandatory Path is the only path.
