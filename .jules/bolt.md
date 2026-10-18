## 2024-05-15 - Statistics Mode uses std::map
**Learning:** `std::map` is used for the `Mode` calculation in `StatisticsEngine::Mode` and this causes allocations, violating Pillar 1 (Zero-Allocation) and it is less performant. Also, `std::map` takes O(N log N) to build and memory allocation per element.
**Action:** Replace `std::map` with an allocation-free copy sort (`std::ranges::sort` on a `Vector` or `FixedVector`) and linear iteration to count consecutive identical elements. This fulfills Pillar 1, and avoids allocation, improving performance.
