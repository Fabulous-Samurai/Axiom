## 2026-08-04 - Optimize ConvertToNumPyMatrix allocation
**Learning:** Found a performance bottleneck where nested vectors were being allocated in `NanobindInterface::ConvertToNumPyMatrix`. Allocating `std::vector<std::vector<double>>` means doing an allocation per row which is extremely slow.
**Action:** Always allocate a single flat vector `std::vector<double>` and use `numpy.reshape(rows, cols)` to construct 2D array without row-by-row allocation overhead.
