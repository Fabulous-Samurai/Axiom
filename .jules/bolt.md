## 2024-05-15 - Zero-Allocation Number Parsing
**Learning:** C++17's `std::from_chars` (and fallback `std::strtod`) natively supports floating-point representations with leading or trailing decimals (e.g., `.5` or `5.`). The codebase unnecessarily created temporary `std::string` copies to pad zeroes before parsing, violating the Zero-Allocation mandate (Zenith Pillar 1).
**Action:** Remove zero-padding string allocation and pass `std::string_view::data()` directly to `std::from_chars`. For fallback, use stack-allocated buffer (up to 64 bytes) with `std::strtod` to avoid exceptions.
