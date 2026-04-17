## 2025-05-18 - [Optimize StatisticsEngine Percentile]
**Learning:** StatisticsEngine methods (e.g., Median, Percentile) use std::nth_element to achieve O(N) average time complexity, replacing O(N log N) full sorts.
**Action:** Replace `std::ranges::sort` with `std::nth_element` in StatisticsEngine methods where a full sort is unnecessary, specifically in `Median` and `Percentile` to improve performance. In `Median` and `Percentile`, only the elements needed for calculations require order, so `std::nth_element` provides a better O(N) complexity compared to O(N log N) full sorts.
