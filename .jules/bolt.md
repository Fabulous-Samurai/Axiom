## 2026-07-07 - Optimize sqlite3 queries by avoiding fetchall()
**Learning:** Calling .fetchall() materializes intermediate rows into a list, causing unnecessary intermediate tuple list allocations which can increase peak memory usage.
**Action:** Always iterate directly over the cursor (e.g. `[r[0] for r in cursor]`) to avoid this allocation overhead.
