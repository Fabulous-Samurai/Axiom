## 2024-10-20 - [Zero-Allocation Mode Calculation]
**Learning:** AXIOM C++ engine must comply with Zenith Pillar 1 (Zero-Allocation), meaning `std::map` and implicit allocations should be avoided. The original mode calculation used `std::map` which performed allocations, violating Pillar 1 and resulting in poor performance.
**Action:** Replaced `std::map` in `StatisticsEngine::Mode` with an allocation-free copy sort (using `std::ranges::sort` on the passed `Vector` by value) and linear iteration to count consecutive identical elements.
