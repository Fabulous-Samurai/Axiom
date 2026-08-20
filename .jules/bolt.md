## 2024-08-20 - Optimizing std::from_chars with a Fast Path
**Learning:** `std::from_chars` does not support leading `+` signs, and relying on `std::string` to handle edge cases like `.5` introduces unnecessary heap allocations for the common case. A fast path attempting direct `std::string_view` parsing (after stripping `+`) avoids allocations for the majority of numbers.
**Action:** Always attempt a zero-allocation fast path first when parsing strings, and only fall back to allocating `std::string` for edge cases. Ensure to strip leading `+` signs manually when using `std::from_chars`.
