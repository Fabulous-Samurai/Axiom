## 2024-05-20 - [Zero-Allocation in FastParseDouble]
**Learning:** C++17 std::from_chars natively supports floating-point representations with leading or trailing decimals (e.g., '.5' or '5.'). Creating temporary std::string copies to pad zeroes is unnecessary overhead and violates zero-allocation constraints.
**Action:** When parsing numeric values on a hot-path, always trust std::from_chars native capabilities rather than allocating temporary string buffers for edge cases.
