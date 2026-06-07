## 2024-06-07 - Optimize SQLite cursor iteration
**Learning:** Using `fetchall()` to materialize intermediate rows into a list before iterating over them is a memory and performance anti-pattern in Python's sqlite3 module. Direct cursor iteration is highly optimized at the C level (`cursor_iternext`) and avoids intermediate list allocations.
**Action:** Use direct cursor iteration in comprehensions (e.g., `[r[0] for r in cursor]`) instead of `fetchall()` to reduce peak memory usage and improve execution time.
