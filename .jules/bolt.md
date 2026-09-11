## 2024-05-24 - Zero-Allocation String to Double
**Learning:** The fast string-to-double parsing in `FastParseDouble` converts `std::string_view` to a heap-allocated `std::string` just to handle missing leading/trailing zeros before calling `std::from_chars`. This violates Zenith Pillar 1 (Zero-Allocation).
**Action:** Use a stack buffer (`char buffer[128]`) to avoid heap allocations for small numeric strings during fast path parsing.
