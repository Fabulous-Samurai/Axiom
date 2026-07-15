## 2026-07-15 - SQLite cursor iteration instead of fetchall()
**Learning:** In Python, iterating over sqlite3 query results using .fetchall() materializes intermediate rows into a list, taking unnecessary memory and time.
**Action:** Use direct cursor iteration (e.g., [r[0] for r in cursor]) to prevent unnecessary intermediate tuple list allocations, reducing peak memory usage.
