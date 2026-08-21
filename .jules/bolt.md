## 2024-08-21 - Optimistic Zero-Allocation with std::from_chars
**Learning:** `std::from_chars` does not support parsing numbers with a leading `+` sign natively, and allocating a `std::string` inside basic parsing helper functions completely violates Zenith Pillar 1 compliance (Zero-Allocation) and tanks performance.
**Action:** Use an optimistic fast-path: attempt a zero-allocation parsing route by manually stripping `+` prefixes from the `std::string_view` before calling `std::from_chars`, and only fall back to allocating a `std::string` for edge cases if the fast-path fails.
