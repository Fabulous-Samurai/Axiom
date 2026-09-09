## 2024-10-24 - Zero-Allocation FastParseDouble with std::from_chars

**Learning:** `std::from_chars` in modern C++17 implementations (GCC 11+, Clang 14+) natively handles floating point string edge cases like `.5` and `5.` that previously required manually appending leading/trailing zeros. However, it explicitly rejects leading `+` signs. Attempting to wrap `std::stod` in a `try/catch` block for edge cases violates AXIOM's Zero-Exception and Zero-Allocation pillars, incurring costly heap allocations (`std::string`) for basic float parsing.

**Action:** When implementing high-performance parsing logic, avoid eagerly copying string views to `std::string`. Pre-process edge cases in-place (e.g., `sv.remove_prefix(1)` for `+`) and utilize bounded stack buffers (e.g., `char buf[64]`) combined with `std::strtod` (checking `errno` and pointer bounds) as a zero-allocation fallback only for legacy compilers where `from_chars` is unavailable for floats.
