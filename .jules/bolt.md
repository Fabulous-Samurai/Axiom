## 2024-09-27 - Zero-Allocation FastParseDouble Optimization
**Learning:** C++17's `std::from_chars` natively supports C locale floating-point parsing for strings with leading or trailing decimals (e.g. `.5` and `5.`). Prepending or appending zeroes using intermediate `std::string` copies is redundant and strictly violates zero-allocation policies.
**Action:** When migrating legacy parser code to `std::from_chars`, directly pass the original `std::string_view` buffers instead of needlessly re-implementing string padding and allocations.
