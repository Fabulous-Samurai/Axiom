## 2024-05-30 - sqlite3 .fetchall() vs Direct Cursor Iteration
**Learning:** In Python's `sqlite3` module, using `.fetchall()` to materialize intermediate rows into a list before iterating over them (e.g., `[r[0] for r in cursor.fetchall()]`) is a memory and performance anti-pattern. Direct cursor iteration in a comprehension (e.g., `[r[0] for r in cursor]`) avoids intermediate tuple list allocations, significantly reducing peak memory usage and improving execution time.
**Action:** Always prefer direct iteration over `cursor` when transforming database rows via list comprehensions.
