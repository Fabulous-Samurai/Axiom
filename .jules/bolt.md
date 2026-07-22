## 2026-07-22 - Avoid .fetchall() for sqlite3 cursors
**Learning:** In Python's sqlite3 module, using .fetchall() materializes intermediate rows into a list of tuples, which can significantly increase peak memory usage when dealing with large datasets.
**Action:** Iterate directly over the cursor (e.g., `[r[0] for r in cursor]`) to prevent unnecessary intermediate tuple list allocations.
