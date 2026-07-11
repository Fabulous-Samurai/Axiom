
## 2026-07-11 - Memory Optimization in SQLite Queries
**Learning:** Using `.fetchall()` on SQLite queries materializes intermediate rows into a list, causing unnecessary peak memory usage, especially for large result sets.
**Action:** Use direct cursor iteration (e.g., `[r[0] for r in cursor]`) to prevent intermediate tuple list allocations.
