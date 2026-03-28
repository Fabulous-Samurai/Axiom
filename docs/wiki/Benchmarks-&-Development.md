# Benchmarks & Development

AXIOM's engineering excellence is backed by rigorous benchmarking and professional development standards.

---

## 8. Benchmarks & Performance Data

### 8.1 Official Benchmarks (v3.1)
Results measured on AMD EPYC 7763 (AVX-512) and Apple M2 (NEON).

| Metric | AVX-512 (Standard) | NEON (Optimized) | notes |
|--------|-------------------|------------------|-------|
| Throughput | 12.4M msg/s | 8.2M msg/s | Single stage |
| Latency (p99) | 850ns | 1.1μs | Minimal routing |
| Memory Overhead | <150MB | <120MB | Idle state |

### 8.2 Hardware Requirements
- **Minimum:** 4-core CPU, 2GB RAM, SSE 4.2 support.
- **Recommended:** 16-core CPU, 8GB RAM, **AVX-512** or **ARMv8.2-A** (LSE) support.

---

## 9. Development & Contribution

### 9.1 Environment Setup
We primarily support **MSYS2 (UCRT64)** for Windows development.

```bash
# Core Toolchain
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
```

### 9.2 Build System & Quality
- **CMake Targets:** 
    - `axiom_engine`: Core compute library.
    - `axiom_dashboard`: Visual monitor.
    - `axiom_studio`: Integrated IDE.
- **SonarQube Standards:** We maintain a **Reliability Rating: A** and zero critical code smells.
- **Testing:** All new features must include `ctest` coverage.

### 9.3 How to Contribute
1. Fork the repository.
2. Create a feature branch (`feat/your-feature`).
3. Ensure SonarQube local analysis passes.
4. Open a Pull Request with a clear technical summary.

---
[← Back to Python API](Python-API.md) | [Next: Roadmap & FAQ →](Roadmap-&-FAQ.md)
