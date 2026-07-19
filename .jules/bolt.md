## 2026-07-19 - Avoid sqlite3 fetchall for single column iteration
**Learning:** When iterating over sqlite3 query results in Python, using `.fetchall()` materializes intermediate rows into a list, which can waste significant memory for large result sets.
**Action:** Use direct cursor iteration (e.g., `[r[0] for r in cursor]`) to prevent unnecessary intermediate tuple list allocations.
