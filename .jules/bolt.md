## 2026-06-05 - SQLite Cursor Iteration Performance
**Learning:** In Python's sqlite3 module, using `.fetchall()` to materialize intermediate rows into a list before iterating over them is a memory and performance anti-pattern. Direct cursor iteration is highly optimized at the C level.
**Action:** Use direct cursor iteration in comprehensions (e.g., `[r[0] for r in cursor]`) to avoid intermediate tuple list allocations and reduce peak memory usage.
