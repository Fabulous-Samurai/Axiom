## 2024-05-23 - Avoid fetchall() in sqlite3 cursor iteration
**Learning:** In Python's sqlite3 module, using `.fetchall()` to materialize intermediate rows into a list before iterating over them (e.g., `[r[0] for r in cursor.fetchall()]`) is a memory and performance anti-pattern. Direct cursor iteration (`for row in cursor`) is highly optimized at the C level (`cursor_iternext`).
**Action:** Use direct cursor iteration in comprehensions (e.g., `[r[0] for r in cursor]`) to avoid intermediate tuple list allocations, significantly reducing peak memory usage and improving execution time.
