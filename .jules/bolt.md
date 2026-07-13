## 2026-07-13 - Avoid SQLite3 .fetchall()
**Learning:** Using .fetchall() on a sqlite3 cursor materializes all intermediate rows into a list in memory before they can be processed into their final format.
**Action:** Use direct cursor iteration (e.g., [r[0] for r in cursor]) instead of .fetchall() to prevent intermediate tuple list allocations, reducing peak memory usage.
