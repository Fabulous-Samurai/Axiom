## 2024-10-07 - Optimize ReplaceAns by replacing std::stringstream with std::to_chars
**Learning:** `std::stringstream` is surprisingly slow for simple formatting of primitive types because it requires allocations, locking in some implementations, and virtual function calls. Using C++17's `std::to_chars` with an on-stack buffer avoids all allocations.
**Action:** Always prefer `std::to_chars` or `fmt::format` for primitive-to-string conversions on hot paths or loops.
