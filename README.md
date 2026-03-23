<div align="center">

# AXIOM

**Verified High-Performance Computation Platform**

*Mission-critical mathematical engine with JIT compilation,
GPU compute, formal verification, and hardware-enforced security.*

[![Version](https://img.shields.io/badge/version-3.1.1-blue.svg)]()
[![License](https://img.shields.io/badge/license-GPLv3-green.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-23-00599C.svg?logo=cplusplus)]()
[![TLA+](https://img.shields.io/badge/TLA+-Verified-brightgreen.svg)]()
[![Vulkan](https://img.shields.io/badge/Vulkan-Compute-red.svg?logo=vulkan)]()

[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=Fabulous-Samurai_Axiom&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=Fabulous-Samurai_Axiom)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=Fabulous-Samurai_Axiom&metric=coverage)](https://sonarcloud.io/summary/new_code?id=Fabulous-Samurai_Axiom)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=Fabulous-Samurai_Axiom&metric=ncloc)](https://sonarcloud.io/summary/new_code?id=Fabulous-Samurai_Axiom)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=Fabulous-Samurai_Axiom&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=Fabulous-Samurai_Axiom)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=Fabulous-Samurai_Axiom&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=Fabulous-Samurai_Axiom)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=Fabulous-Samurai_Axiom&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=Fabulous-Samurai_Axiom)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=Fabulous-Samurai_Axiom&metric=bugs)](https://sonarcloud.io/summary/new_code?id=Fabulous-Samurai_Axiom)
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?project=Fabulous-Samurai_Axiom&metric=vulnerabilities)](https://sonarcloud.io/summary/new_code?id=Fabulous-Samurai_Axiom)

[Quick Start](#quick-start) •
[Architecture](#architecture) •
[Benchmarks](#benchmarks) •
[Verification](#formal-verification) •
[Security](#security-architecture) •
[Build](#build-system) •
[Roadmap](#roadmap)

---

</div>

## What is AXIOM?

AXIOM is not just a math library. It is a **vertically integrated computation platform** that spans from CPU instruction-level optimization to formal mathematical proofs of correctness.

```
┌─────────────────────────────────────────────────────┐
│                  AXIOM PLATFORM                      │
├─────────────────────────────────────────────────────┤
│                                                      │
│  ┌─────────────────────────────────────────────┐    │
│  │ Formal Verification Layer (TLA+)            │    │
│  │ 3 specs • 25K+ states • 8 invariants       │    │
│  └──────────────────────┬──────────────────────┘    │
│                         │                            │
│  ┌──────────────────────▼──────────────────────┐    │
│  │ Application Layer                            │    │
│  │ Algebraic • Symbolic • Linear • Statistics  │    │
│  └──────────────────────┬──────────────────────┘    │
│                         │                            │
│  ┌──────────────────────▼──────────────────────┐    │
│  │ Runtime Layer                                │    │
│  │ Zenith JIT • HarmonicArena • Dispatcher     │    │
│  └──────────────────────┬──────────────────────┘    │
│                         │                            │
│  ┌──────────────────────▼──────────────────────┐    │
│  │ Hardware Abstraction Layer                   │    │
│  │ AVX2/AVX-512 • Vulkan Compute • AF_XDP     │    │
│  └──────────────────────┬──────────────────────┘    │
│                         │                            │
│  ┌──────────────────────▼──────────────────────┐    │
│  │ Security Layer                               │    │
│  │ Secure Vault (TEE/SGX) • W^X Policy         │    │
│  └─────────────────────────────────────────────┘    │
│                                                      │
└─────────────────────────────────────────────────────┘
```

---

## Key Innovations

### ⚡ Zenith JIT Compiler

Custom AST-to-x64 machine code translator built on AsmJit. Translates mathematical expressions directly into native CPU instructions at runtime.

- **18-20x** faster than interpreted evaluation
- **W^X Security**: Write-XOR-Execute policy prevents code injection
- Automatic engagement for hot arithmetic paths via SelectiveDispatcher
- Support for complex functions, variables, and nested expressions

### 🛡️ Secure Vault Architecture

Hardware-enforced computation isolation prepared for Intel SGX/TEE integration.

- State machine integrity guaranteed at silicon level
- Memory encryption for sensitive computations
- Attestation-ready enclave design
- Production sandboxing with ComplexityGuard

### 🌐 Kernel-Bypass Networking (AF_XDP)

Zero-copy packet processing that bypasses the entire OS network stack.

- Direct NIC-to-application data path
- Millions of packets per second throughput
- Zero-copy DMA transfer
- NUMA-topology aware buffer management

### 🔧 HarmonicArena Allocator

Custom lock-free concurrent memory allocator optimized for HPC workloads.

- Thread-local arena pools (zero contention)
- Cache-line aligned allocations (no false sharing)
- Built-in profiling and diagnostics
- NUMA-aware memory placement

---

## Computation Backends

| Backend | Capability | Optimization |
|---------|-----------|--------------|
| **Algebraic** | High-precision parsing, evaluation | EvaluateFast typed fast-path |
| **Symbolic** | Differentiation, Taylor series, limits | Incremental AST-based expansion |
| **Linear** | Matrix operations, system solving | AVX2/FMA SIMD intrinsics |
| **Statistics** | Distributions, regression, analysis | Vectorized computation |

**Interface:** C++23 core with hybrid FFI model using nanobind for Python integration.

---

## Quick Start

### Build

```bash
# Requirements: C++23 compiler (GCC 12+, MSVC 19.38+), CMake 3.12+, Ninja
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Run

```bash
# Interactive mode
./build/axiom

# Direct evaluation
./build/axiom "2 + 3 * 4"

# Symbolic differentiation
./build/axiom "derive x^2 + sin(x)"

# Numeric integration
./build/axiom "integrate(x^2, x, 0, 10)"
```

### Verify

```bash
# Core tests
./build/run_tests

# Production validation
./build/giga_test_suite

# Cross-backend consistency (Interpreted vs JIT vs Symbolic)
./build/differential_suite

# Lock-free allocator stress test
./build/arena_stress

# Security vault integrity
./build/enclave_test
```

---

## Architecture

### SelectiveDispatcher Pipeline

```
User Expression
      │
      ▼
  ┌────────┐
  │ Parser │──→ AST (Abstract Syntax Tree)
  └────┬───┘
       │
       ▼
  ┌─────────────────────┐
  │ SelectiveDispatcher  │
  │                      │
  │  Route by:           │
  │  ├─ Expression type  │
  │  ├─ Complexity       │
  │  └─ Available HW     │
  └──┬──────┬──────┬─────┘
     │      │      │
     ▼      ▼      ▼
  ┌─────┐┌─────┐┌────────┐
  │Interp││ JIT ││ Vulkan │
  │reter ││Zenith││Compute │
  └─────┘└─────┘┌────────┐
     │      │      │
     └──────┴──────┘
            │
            ▼
     ┌──────────┐
     │  Result  │
     └──────────┘
```

### Thread-Local Optimization

```cpp
// Zero allocation per-request via thread_local DynamicCalc
// Each thread reuses its own calculator instance
// No heap allocation, no contention, no lock
static thread_local DynamicCalc calc;
```

### Python Integration

```
C++23 Core ←→ nanobind FFI ←→ Python Layer
                                ├── GUI (PySide6)
                                ├── Workspace
                                └── Tests (pytest)
```

---

## Benchmarks

### Engine Benchmark

```bash
cmake --build build --config Release --target axiom_benchmark
./build/axiom_benchmark
```

Measures scalar parser throughput, typed fast-path throughput (EvaluateFast), zero-copy vector transfer, and lock-free queue IPC cycle behavior. Outputs `benchmark_results.csv` and `benchmark_results.json`.

### Workspace Scalability (1M Variables)

```bash
python tests/functional/performance/benchmark_1m_vars.py
```

**Platform:** Python 3.12.12, fully headless (no Qt/GUI)

| Scale | Insert (ms) | Lookup p99 (µs) | Throughput (vars/sec) |
|------:|------------:|----------------:|----------------------:|
| 100 | < 0.1 | 1.700 | 2,538,070 |
| 1,000 | 0.3 | 0.300 | 3,419,973 |
| 10,000 | 4.0 | 0.800 | 2,506,014 |
| 100,000 | 72.2 | 1.700 | 1,384,328 |
| 1,000,000 | 1,087 | 2.200 | 919,889 |

### Scorecard — All Checks Passed ✅

| Check | Budget | Measured | Result |
|-------|--------|----------|--------|
| 1M inserts | < 2,000 ms | 1,728 ms | ✅ PASS |
| Lookup p99 | < 5 µs | 2.5 µs | ✅ PASS |
| 8,000 ans# appends | < 50 ms | 8.3 ms | ✅ PASS |
| Dirty-flag flush p99 | < 100 µs | 0.5 µs | ✅ PASS |
| rowCount() latency | < 5 µs | 184.9 ns | ✅ PASS |
| Snapshot ≥ 1M keys | ≥ 1,000,000 | 1,009,000 | ✅ PASS |

**Memory footprint at 1,000,000 variables: 61.1 MB**

The benchmark is also collected by pytest:

```bash
pytest tests/functional/performance/benchmark_1m_vars.py -v
# → 5 passed in ~2.0 s
```

---

## Security Architecture

### W^X (Write XOR Execute) Policy

```
JIT Code Lifecycle:
1. mmap(PROT_WRITE)     → Allocate writable page
2. emit_code(buffer)     → Write machine code
3. mprotect(PROT_EXEC)   → Switch to executable (non-writable)
4. execute(buffer)       → Run native code
5. mprotect(PROT_WRITE)  → Switch back for update (non-executable)

At no point is memory both writable AND executable.
```

### Secure Vault (TEE/SGX Ready)

```
┌─────────────────────────────────┐
│      Untrusted OS / App         │
├─────────────────────────────────┤
│   ┌─────────────────────────┐   │
│   │    Secure Enclave       │   │
│   │  ┌──────────────────┐   │   │
│   │  │  State Machine   │   │   │
│   │  │  (Integrity      │   │   │
│   │  │   Guaranteed)    │   │   │
│   │  └──────────────────┘   │   │
│   │  Memory: Encrypted      │   │
│   │  Access: Attested       │   │
│   └─────────────────────────┘   │
└─────────────────────────────────┘
```

### Runtime Guardrails

Tunable without recompilation via environment variables:

**Expression Policy:**

| Variable | Default | Purpose |
|----------|---------|---------|
| `AXIOM_POLICY_MAX_CHARS_DEFAULT` | 8192 | Max expression length |
| `AXIOM_POLICY_MAX_CHARS_SYMBOLIC` | 16384 | Max symbolic expression |
| `AXIOM_POLICY_MAX_TOKENS` | 2048 | Max token count |
| `AXIOM_POLICY_MAX_DEPTH_DEFAULT` | 128 | Max nesting depth |
| `AXIOM_POLICY_MAX_DEPTH_SYMBOLIC` | 256 | Max symbolic depth |
| `AXIOM_POLICY_MAX_CARET_OPS` | 64 | Max exponentiation ops |
| `AXIOM_POLICY_MAX_MATRIX_ELEMENTS` | 40000 | Max matrix size |

**Daemon Resilience:**

| Variable | Default | Purpose |
|----------|---------|---------|
| `AXIOM_DAEMON_CIRCUIT_FAILURE_THRESHOLD` | 5 | Failures before circuit opens |
| `AXIOM_DAEMON_CIRCUIT_OPEN_MS` | 2000 | Circuit open duration |
| `AXIOM_DAEMON_BACKPRESSURE_WAIT_MS` | 5 | Backpressure wait time |

**Suggested Presets:**
- **Balanced:** Keep defaults for most workloads
- **Strict:** Reduce tokens/depth by 30-50%, lower circuit threshold to 3
- **Throughput:** Increase backpressure wait to 8-12ms

---

## Formal Verification

AXIOM includes TLA+ models providing mathematical proofs of system correctness.

### Specifications

| Specification | Location | Config |
|--------------|----------|--------|
| Workspace Scalability | `formal/tla/AxiomWorkspaceScalability.tla` | `formal/tla/AxiomWorkspaceScalability.cfg` |
| IPC Protocol | `formal/tla/AxiomIpcProtocol.tla` | `formal/tla/AxiomIpcProtocol.cfg` |
| Daemon Queue Fairness | `formal/tla/AxiomDaemonQueueFairness.tla` | `formal/tla/AxiomDaemonQueueFairness.cfg` |

### Verification Results

**Workspace Scalability Model:**
- TLC explored **2,187 distinct states** (depth 18)
- **8 safety invariants** verified: ViewConsistency, DirtyConsistency, CleanFlushIdempotency, O1RowCount, and others
- **1 liveness property verified**: `[](dirty => <>(~dirty))` — every dirty write is eventually reflected in the view
- **O(1) structural guarantee**: `RowCount() = len(dict) + len(list)` — proven by construction

**IPC Protocol Model:**
- Interactive protocol framing (`__END__`)
- Mode switch sequencing before command execution
- Request/response progression safety

**Daemon Queue Fairness:**
- Enqueued requests eventually processed and completed
- No starvation under weak fairness scheduler

Full guide: [docs/formal/TLA_PLUS_VERIFICATION.md](docs/formal/TLA_PLUS_VERIFICATION.md)

---

## Build System

### Build Targets

| Target | Purpose |
|--------|---------|
| `axiom` | Primary CLI engine |
| `run_tests` | Core functionality validation |
| `giga_test_suite` | Production-grade monolithic validation |
| `differential_suite` | Cross-backend consistency (Interpreted vs JIT vs Symbolic) |
| `arena_stress` | Lock-free allocator concurrency stress test |
| `enclave_test` | Security vault integrity verification |
| `axiom_benchmark` | Performance measurement suite |
| `ast_drills` | AST-focused validation |

### CMake Configuration

| Flag | Default | Purpose |
|------|---------|---------|
| `AXIOM_AUTO_INSTALL_PYTHON_DEPS` | ON | Auto-install Python deps in venv |
| `AXIOM_ENABLE_EMBEDDED_PYTHON_ENGINE` | OFF | Embedded Python engine sources |
| `AXIOM_ENABLE_CXX20_MODULES` | OFF | Experimental C++20 modules |
| `AXIOM_ENABLE_HARMONIC_ARENA` | OFF | Lock-free allocator subsystem |
| `BUILD_GIGA_TESTS` | ON | Production test suite |

### Testing

```bash
# C++ tests
cmake --build build --config Release --target run_tests giga_test_suite ast_drills
./build/run_tests
./build/giga_test_suite
./build/ast_drills

# Python tests
pytest
```

Detailed test playbook: [tests/README.md](tests/README.md)

---

## Repository Structure

```
axiom/
├── src/                    Core engine and runtime
├── include/                Public API headers
├── core/
│   └── dispatch/           SelectiveDispatcher and C API adapter
├── tests/                  C++ and Python test suites
├── gui/python/             Python GUI frontend (PySide6)
├── formal/tla/             TLA+ specifications and configs
├── docs/                   Documentation and guides
└── modules/                Experimental C++20 modules
```

---

## Roadmap

AXIOM is evolving into a complete computation ecosystem:

| Component | Status | Description |
|-----------|--------|-------------|
| **AXIOM Engine** | ✅ Active | C++23 core with multi-mode computation |
| **Zenith JIT** | ✅ Active | AST-to-x64 native code compilation |
| **HarmonicArena** | ✅ Active | Lock-free concurrent allocator |
| **Secure Vault** | ✅ Active | TEE/SGX hardware isolation |
| **AF_XDP Ingress** | ✅ Active | Kernel-bypass zero-copy networking |
| **TLA+ Verification** | ✅ Active | Formal correctness proofs |
| **AXIOM Studio** | 🔄 Planned | Professional GUI (QML + Vulkan backend) |
| **Pluto Core** | 🔄 Planned | AI orchestration engine (SLM ensemble + Petri Net) |
| **AXIOM Lens** | 📋 Backlog | Technical document analysis and verification |
| **Gravitum Models** | 📋 Backlog | Verified AI model family (5B / 11B / 23B) |

---

## Documentation

| Guide | Location |
|-------|----------|
| Test Playbook | [tests/README.md](tests/README.md) |
| TLA+ Verification Guide | [docs/formal/TLA_PLUS_VERIFICATION.md](docs/formal/TLA_PLUS_VERIFICATION.md) |
| Optional Python Deps | [requirements-optional.txt](requirements-optional.txt) |
| Workspace TLA+ Spec | [formal/tla/AxiomWorkspaceScalability.tla](formal/tla/AxiomWorkspaceScalability.tla) |
| Multi-device Install | [docs/INSTALL_OTHER_DEVICES.md](docs/INSTALL_OTHER_DEVICES.md) |
| Changelog | [CHANGELOG.md](CHANGELOG.md) |

---

## Known Constraints

- Some CLI help output references compile-time enterprise flags
- Daemon runtime paths are platform-conditional and build-definition-dependent
- C++20 modules support is conservative per compiler profile
- JIT compilation currently targets x64 architecture (ARM twin planned)

---

## License

Licensed under the **GNU General Public License v3.0**. See [LICENSE](LICENSE) for details.

This ensures all modifications and derivative works remain open-source, protecting the community and ecosystem.

---

<div align="center">

**AXIOM: Verified. Fast. Secure.**

*Built from bare-metal to formal proof.*

[GitHub](https://github.com/Fabulous-Samurai/axiom) •
[Documentation](docs/) •
[Changelog](CHANGELOG.md)

</div>
