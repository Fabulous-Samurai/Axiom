## 2024-10-26 - std::map vs In-Place Sorting for Mode Calculation
**Learning:** Using `std::map` to compute the statistical mode is surprisingly slow for large arrays, because it performs numerous heap allocations per unique entry (O(N log N) with heavy constant overhead).
**Action:** Replace `std::map` with an allocation-free copy sort (like `std::ranges::sort` on `Vector`) and linear iteration to count consecutive identical elements. This is significantly faster and aligns with Zenith Pillar 1 (Zero-Allocation).
