## 2024-05-24 - Python sqlite3 iteration
**Learning:** When iterating over `sqlite3` query results in Python, using `.fetchall()` materializes intermediate rows into a list, increasing peak memory usage.
**Action:** Use direct cursor iteration (e.g., `[r[0] for r in cursor]`) to prevent unnecessary intermediate tuple list allocations.
