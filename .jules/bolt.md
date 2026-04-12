## 2025-04-11 - [Optimize BoxPlot Quartile Extraction]
**Learning:** Found that `PlotEngine::BoxPlot` used `std::sort` (O(N log N)) to extract min, max, median, Q1, and Q3. This was an unneeded bottleneck.
**Action:** Replaced `std::sort` with `std::minmax_element` and `std::nth_element` to achieve O(N) extraction. Add explicit boundary checks (e.g. `q1_idx < med_idx`) to prevent UB for small arrays when partitioning sub-ranges.
