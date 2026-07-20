## 2026-07-20 - Zero-Allocation Mode Calculation
**Learning:** Using `std::map` for frequency counting in core computational methods violates Zenith Pillar 1 (Zero-Allocation) and drastically slows down performance due to heap allocations.
**Action:** Replace map-based counting with an in-place `std::ranges::sort` and O(N) scan. This requires passing `Vector` by value instead of const reference to mutate the local copy without explicitly allocating a new vector.
