
## 2026-07-09 - Memory Optimization for SQLite queries
**Learning:** Using `.fetchall()` on a sqlite3 cursor materializes all rows into a list of tuples in memory, which significantly increases peak memory usage for large queries.
**Action:** Iterate directly over the cursor (e.g., `[r[0] for r in cursor]`) to avoid intermediate tuple list allocations.
