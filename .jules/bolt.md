## YYYY-MM-DD - Remove std::string copy in FastParseDouble
**Learning:** C++17's `std::from_chars` natively supports floating-point representations with leading or trailing decimals (like `.5` or `5.`), making temporary string allocations for padding zeroes completely unnecessary.
**Action:** Avoid preemptive zero-padding logic and temporary `std::string` copies when using `std::from_chars` for numeric parsing, adhering strictly to Zero-Allocation policies.
