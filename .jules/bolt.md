## 2025-01-24 - Zero-Exception String Parsing
**Learning:** Using `std::stod` inside a `try/catch` block for string-to-number parsing (especially in frequent validation paths like `IsNumber`) causes severe performance regressions due to C++ exception overhead on failure paths. This violates Zenith Pillar 5: Zero-Exception & No RTTI.
**Action:** Use `std::strtod` (with `errno=0` and proper bounds checking) as a high-performance, exception-free fallback instead.
