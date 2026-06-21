## 2024-06-21 - Standard Library Allocation Violations
**Vulnerability:** Found uses of `std::map` and `std::string` inside AXIOM core processing directories (`engine/compute/statistics_engine.cpp`), which violates Zenith Pillar 1/3 (Zero-Allocation policy).
**Learning:** Using default standard library containers (`std::map`, `std::string`) triggers heap allocations and unpredictable GC pauses. This creates a Denial of Service vulnerability in real-time embedded environments where memory and timing must be tightly controlled.
**Prevention:** Always use `tsl::robin_map` (or Arena-based structures) for hash maps and `std::string_view` (or pre-allocated char arrays) for strings to comply with the Zero-Allocation policy and ensure RDTSC Determinism.
