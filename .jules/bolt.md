
## $(date +%Y-%m-%d) - [O(N) Complexity for Statistical Methods]
**Learning:** Replaced $O(N \log N)$ full sorting algorithms (`std::sort`, `std::ranges::sort`) with $O(N)$ partitioning algorithms (`std::nth_element`, `std::minmax_element`) for operations that only need quartile or positional data (e.g., `Median`, `Percentile`, `BoxPlot`).
**Action:** When working on statistical extractions in arrays where only subsets or single elements are needed, avoid full array sorting and opt for C++ standard partitioning and bounding functions. Ensure edge-case safeguards when subdividing parts of the array (e.g., bounds checking `if (q1_idx < med_idx)`).
