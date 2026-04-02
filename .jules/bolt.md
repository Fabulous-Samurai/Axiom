## 2024-04-02 - [Zero-Allocation Mode Calculation]
**Learning:** Using `std::map` in hot-path statistical functions (like `StatisticsEngine::Mode`) causes unnecessary heap allocations and cache misses, violating Zenith Pillar 1.
**Action:** Use sorted vectors (e.g. `AXIOM::FixedVector`) and linear scans for frequency counting in core modules to adhere to zero-allocation pillars.
