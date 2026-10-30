## 2023-10-27 - [Title]
**Learning:** `std::map` and `std::unordered_map` allocate per-node, violating zero-allocation constraints and causing significant overhead in C++ engines compared to vector sorting. By changing `Mode(const Vector& data)` to `Mode(Vector data)` to pass-by-value, we can mutate the copy in-place (sorting it) and iterate linearly to calculate the mode without any heap allocations. Passing by value allows the compiler to optimize rvalue arguments entirely.
**Action:** Replace map-based mode calculation with vector sorting and pass-by-value signature to optimize `Mode()`.
