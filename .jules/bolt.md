## 2026-08-15 - Zero-allocation parsing via std::from_chars on string_view
**Learning:** Parsing numbers directly from string_view buffer with std::from_chars prevents O(N) std::string heap allocation. Because std::from_chars might not handle leading +, or naked decimals like '.5', an optimistic fast-path ensures speed without regressions.
**Action:** Always attempt zero-allocation paths on string_view first and fallback to allocations only if parsing fails or needs special handling.
