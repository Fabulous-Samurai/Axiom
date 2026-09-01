
## 2024-05-30 - Zero-Allocation Fast Parse
**Learning:** C++17's `std::from_chars` natively parses `.5` and `5.` floating point numbers without requiring manual zero-padding. Furthermore, using `std::strtod` avoids exceptions and severe performance regressions for fallback parsings on un-supported environments.
**Action:** Avoid allocating strings and string padding for numeric parsers to meet Zenith Pillar zero-allocation. Replace `std::stod` inside try-catch with `std::strtod` to maintain zero-exception compliance.
