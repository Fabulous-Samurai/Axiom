## 2026-08-05 - Flat Vectors for FFI
**Learning:** Using nested std::vector<std::vector<double>> to convert Eigen::Matrix to NumPy arrays causes O(rows) memory allocations, introducing severe allocation overhead.
**Action:** Allocate a single flat std::vector<double> of size rows*cols, pass it to NumPy, and use .reshape(rows, cols) to recreate the 2D structure.
