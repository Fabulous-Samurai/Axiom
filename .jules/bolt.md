## 2026-08-08 - Avoiding O(N) Allocations in Nanobind Matrix Conversion
**Learning:** Using nested std::vector<std::vector<double>> to convert an Eigen::Matrix to a 2D NumPy array via nanobind incurs O(rows) separate memory allocations. This is highly inefficient compared to flattening.
**Action:** Always allocate a single flat std::vector<double> of size rows * cols, copy the elements, and then use np.attr('array')(flat, 'dtype'_a = 'float64').attr('reshape')(rows, cols) to return the matrix.
