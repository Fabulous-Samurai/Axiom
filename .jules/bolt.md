## YYYY-MM-DD - Optimize MovingAverage Algorithm
**Learning:** Found a nested loop O(N * W) calculating MovingAverage in StatisticsEngine::MovingAverage where N is the length of data and W is window size.
**Action:** Replaced it with an O(N) sliding window approach by keeping a running sum, reducing algorithm time complexity.
