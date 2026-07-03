## 2024-05-18 - Avoid fetchall() on SQLite cursors
**Learning:** When iterating over `sqlite3` query results in Python, using `.fetchall()` materializes intermediate rows into a list of tuples. This can cause unnecessary memory allocations and CPU overhead for large datasets.
**Action:** Instead of `.fetchall()`, use direct cursor iteration (e.g., `[r[0] for r in cursor]`) to prevent intermediate tuple list allocations, reducing peak memory usage and improving performance.
