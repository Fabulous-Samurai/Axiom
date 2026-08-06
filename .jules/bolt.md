## 2024-05-24 - Initial Journal
**Learning:** Initial journal setup.
**Action:** None.
## 2026-08-06 - Avoid Nested std::vector for NumPy Matrix Conversions
**Learning:** Using `std::vector<std::vector<double>>` to create a NumPy matrix via nanobind incurs severe memory allocation overhead (O(rows) allocations).
**Action:** Always allocate a single flat `std::vector<double>` of size `rows * cols` and use `.attr("reshape")(rows, cols)` on the NumPy array object to minimize allocations to O(1).
