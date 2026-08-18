## 2026-08-18 - Zero-allocation string-to-double parsing
**Learning:** Using `std::string(std::string_view)` unconditionally for simple number parsing introduces unnecessary heap allocations for the vast majority of well-formatted numbers.
**Action:** Implement an optimistic fast-path using `std::from_chars` directly on the `std::string_view` first, and only fall back to allocating a `std::string` if edge cases (like leading `+` or missing trailing/leading zeroes) are encountered.
