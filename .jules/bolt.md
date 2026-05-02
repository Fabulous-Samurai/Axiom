
## 2024-05-24 - O(N) Order Statistics in C++
**Learning:** For computing order statistics (like medians or percentiles) in C++ in O(N) time without full sorting, `std::nth_element` is highly effective. Crucially, when an adjacent element is required (e.g., the preceding element for even-sized medians, or the upper bound for percentiles), we don't need a full sort. We can efficiently find it by applying `std::max_element` or `std::min_element` to the appropriate partitioned subrange.
**Action:** Always favor `std::nth_element` combined with `std::min_element`/`std::max_element` over `std::ranges::sort` or `std::sort` when implementing median, percentile, or other order statistic algorithms to avoid unnecessary O(N log N) overhead.
