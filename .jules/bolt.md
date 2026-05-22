## 2024-05-24 - [Memory anti-pattern in sqlite3]
**Learning:** In Python's sqlite3 module, using `.fetchall()` to materialize intermediate rows into a list before iterating over them is a memory and performance anti-pattern. Direct cursor iteration is highly optimized at the C level.
**Action:** Always use direct cursor iteration (e.g. `[r[0] for r in cursor]`) instead of `fetchall()` (e.g. `[r[0] for r in cursor.fetchall()]`) to avoid intermediate tuple list allocations.
