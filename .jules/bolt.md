## 2026-07-05 - Avoid fetching all SQLite rows into memory
**Learning:** When iterating over sqlite3 query results in Python, using .fetchall() materializes intermediate rows into a list, which can allocate significant memory and cause peak memory usage spikes, especially for large result sets.
**Action:** Avoid using .fetchall(). Instead, use direct cursor iteration (e.g., [r[0] for r in cursor]) to prevent unnecessary intermediate tuple list allocations.
