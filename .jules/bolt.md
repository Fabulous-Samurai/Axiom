## 2026-08-12 - Optimize Matrix to NumPy conversion by removing nested vector allocations
**Learning:** Using nested `std::vector<std::vector<double>>` to pass 2D arrays across FFI boundaries (nanobind) incurs O(rows) heap allocations.
**Action:** Use a flat `std::vector<double>` of size `rows * cols` and use numpy's reshape on the resulting array to eliminate unnecessary per-row heap allocations.
