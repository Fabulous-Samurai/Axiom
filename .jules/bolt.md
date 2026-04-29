## 2024-05-18 - [Optimize Order Statistics]
**Learning:** `std::nth_element` provides an O(N) solution for order statistics (median, percentile) replacing O(N log N) full sorting. It partitions the array such that the Nth element is in its sorted position and all elements before it are less than or equal to it. To find the (N-1)th element, you can simply use `std::max_element` on the left partition.
**Action:** Always prefer `std::nth_element` over `std::sort` when calculating single quantiles like Median, Percentiles or quartiles.
