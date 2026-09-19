## 2024-05-24 - std::from_chars natively supports edge case padding
**Learning:** Manual zero padding (e.g. padding ".5" to "0.5") via `std::string` copies prior to parsing numeric strings is redundant when using C++17 `std::from_chars`, and violates Zero-Allocation policies by incurring string allocations on every parse attempt.
**Action:** Rely on standard parsing library capabilities to process floating-point edge cases naturally, and remove string temporaries from hot-path parsers to guarantee strictly zero allocations.
