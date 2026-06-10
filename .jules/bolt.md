## 2024-06-10 - Replace .fetchall() with Cursor Iteration
**Learning:** In Python's `sqlite3` module, using `.fetchall()` to materialize intermediate rows into a list before iterating over them is a memory and performance anti-pattern. Direct cursor iteration in a comprehension avoids intermediate tuple list allocations.
**Action:** Use direct cursor iteration in a comprehension (e.g., `[r[0] for r in cursor]`) instead of `.fetchall()`.
