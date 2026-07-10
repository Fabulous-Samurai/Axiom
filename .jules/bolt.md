## 2026-07-10 - Optimize sqlite3 fetch overhead
**Learning:** Using `.fetchall()` on a sqlite3 cursor materializes all rows into an intermediate list of tuples before they are consumed, which unnecessarily increases peak memory usage for large result sets.
**Action:** Iterate directly over the cursor (e.g., `[r[0] for r in cursor]`) to stream results and prevent unnecessary list allocations.
