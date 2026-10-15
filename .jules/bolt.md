
## 2024-10-15 - Zero-Allocation Mode Calculation
**Learning:** Found an `std::map` being used in `StatisticsEngine::Mode` to calculate the mode of a dataset. In this codebase's architecture, memory allocations like `std::map` violate Zenith Pillar 1 (Zero-Allocation), causing deterministic latency issues and cache misses.
**Action:** Always favor an allocation-free copy sort (`std::ranges::sort` on `FixedVector`) and linear iteration to count consecutive elements for frequency analysis, avoiding heap allocations completely.
