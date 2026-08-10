## 2026-08-10 - Optimize Matrix to NumPy Conversion
**Learning:** Nested `std::vector<std::vector<double>>` structures in nanobind matrix conversions introduce severe memory allocation overhead (O(rows) allocations).
**Action:** Always allocate a flat `std::vector` of size `rows * cols` and use NumPy's reshape operation via `.attr("reshape")` to return 2D arrays efficiently with a single O(1) allocation.
