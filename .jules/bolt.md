## 2026-08-07 - Avoid nested std::vector allocations in Nanobind NumPy conversions
**Learning:** Converting Eigen::Matrix to NumPy arrays using nested `std::vector<std::vector<double>>` introduces severe memory allocation overhead (O(rows) allocations).
**Action:** Allocate a flat `std::vector<double>` of size `rows * cols` and use `np.attr('array')(flat, 'dtype'_a = 'float64').attr('reshape')(rows, cols)` to return the 2D array efficiently.
