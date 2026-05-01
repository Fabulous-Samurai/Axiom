## 2024-05-18 - Replacing sort with nth_element
**Learning:** Found an opportunity to replace `std::sort` with `std::nth_element` + `std::max_element` in `Median` and `Percentile` implementations for a complexity reduction from $O(N \log N)$ to $O(N)$.
**Action:** Always check median/percentile code to see if full sorting is used and replace with partitioning for linear time efficiency.
