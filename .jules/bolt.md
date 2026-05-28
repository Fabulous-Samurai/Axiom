## 2026-05-28 - [SQLite Cursor Iteration]
**Learning:** Using `.fetchall()` to materialize intermediate rows into a list before iterating over them is a memory and performance anti-pattern in Python's sqlite3 module. Direct cursor iteration is highly optimized at the C level.
**Action:** Use direct cursor iteration in a comprehension (e.g., `[r[0] for r in cursor]`) instead of `fetchall()` to significantly reduce peak memory usage and improve execution time.
