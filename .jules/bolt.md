## 2024-05-05 - Replace std::sort with std::nth_element in plot_engine.cpp
**Learning:** Found a performance bottleneck in `PlotEngine::BoxPlot` where an $O(N \log N)$ `std::sort` was used to find quantiles (min, max, median, Q1, Q3). Boxplot statistics can be computed in linear $O(N)$ time.
**Action:** Replaced `std::sort` with partition-based `std::nth_element`, `std::min_element`, and `std::max_element` to achieve significant algorithmic performance improvement for large datasets while maintaining identical behavior.
