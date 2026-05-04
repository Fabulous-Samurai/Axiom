## 2024-05-04 - [Optimize cache eviction loop]
**Learning:** For performance, prefer direct dictionary iteration (`for key in d:`) over `tuple(d.keys())` to avoid unnecessary allocations during eviction cycles.
**Action:** Always use direct dict iteration for performance-critical loops.
