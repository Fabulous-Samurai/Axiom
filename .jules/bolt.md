## 2026-07-12 - SQLite Cursor Iteration
**Learning:** Using `.fetchall()` on an SQLite cursor in Python materializes the entire result set into a list of tuples in memory, causing unnecessary intermediate allocations and increased peak memory usage.
**Action:** Iterate directly over the cursor (e.g., `[r[0] for r in cursor]`) instead of using `.fetchall()` to avoid temporary list allocations.
