## 2026-07-14 - sqlite3 fetchall intermediate allocation
**Learning:** Using .fetchall() on sqlite3 cursors followed by a list comprehension creates an unnecessary intermediate list of tuples, increasing peak memory usage and CPU time.
**Action:** Use direct cursor iteration in a list comprehension (e.g. [r[0] for r in cursor]) instead of .fetchall() to avoid the intermediate list allocation.
