## 2024-06-25 - AXIOM::Vector limitations
**Learning:** `AXIOM::Vector` is an alias for `AXIOM::FixedVector<double, 256>` to comply with the project's zero-allocation (Zenith Pillar) constraints. It lacks some standard `std::vector` methods, such as `.back()`. When writing or modifying algorithms, you must manually access the last element using `data[data.size() - 1]` instead of `data.back()`.
**Action:** Always check the definition of `AXIOM::FixedVector` in `engine/include/fixed_vector.h` before using standard STL vector methods, and prefer index-based access where STL methods are missing.
