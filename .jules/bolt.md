## 2024-05-18 - [Optimization] BoxPlot Performance
**Learning:** In C++ computing percentiles/median does not require sorting the entire array, which takes O(N log N). Standard algorithms `std::nth_element` with `std::min_element`/`std::max_element` can be used to isolate the required order statistics in O(N) time. In `PlotEngine::BoxPlot`, replacing `std::sort` significantly improves the performance of large plots.
**Action:** When computing order statistics like quantiles or medians, use `std::nth_element` instead of `std::sort` for O(N) time complexity. Ensure correct partition slicing.
