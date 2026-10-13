## 2024-05-15 - Replace std::map with sorting for Mode calculation
**Learning:** For frequency analysis or mode calculations in the AXIOM C++ engine, `std::unordered_map` and `std::map` perform poorly due to constant hash table allocations on the target architecture. This violates Zenith Pillar 1 (Zero-Allocation).
**Action:** Always favor an allocation-free copy sort (e.g., `std::ranges::sort` on a `FixedVector` or regular `Vector`) and linearly iterate to count consecutive identical elements to satisfy Zenith Pillar 1 and achieve optimal performance.
