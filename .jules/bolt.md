## 2024-10-14 - Zero-Allocation Frequency Analysis
**Learning:** In the AXIOM C++ engine, `std::map` and `std::unordered_map` perform constant heap allocations which violate the zero-allocation Zenith Pillar mandate and degrade performance significantly during frequency analysis (e.g., statistical mode).
**Action:** Replace map-based frequency counting with a copy-sort approach using `std::ranges::sort` on allocation-free vectors (like `AXIOM::FixedVector`), then iterating linearly to find the mode in O(N log N) time with O(1) extra space and zero allocations.
