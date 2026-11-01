## 2024-11-01 - Optimizing Frequency Analysis Allocation
**Learning:** `std::map` performs constant heap allocations which violates the Zero-Allocation Zenith Pillar, making it unsuitable for mode calculations in high-performance engines.
**Action:** Replace `std::map` usage with an allocation-free copy sort (`std::ranges::sort`) on a pass-by-value `Vector`, and linearly iterate to count consecutive identical elements. Ensure to use `std::isnan` check before sorting to prevent strict weak ordering violations.
