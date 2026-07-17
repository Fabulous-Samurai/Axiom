## 2026-07-17 - O(N*W) Nested Loop in StatisticsEngine::MovingAverage
**Learning:** The C++ backend implementation of StatisticsEngine::MovingAverage used an O(N*W) nested loop, which becomes a severe bottleneck for large arrays and large window sizes.
**Action:** Always check array aggregation functions (like moving averages, sliding windows) for naive nested loops and replace them with O(N) sliding window algorithms (initial sum + sliding delta).
