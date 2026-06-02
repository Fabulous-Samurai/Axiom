## 2024-05-24 - [Optimize SQLite fetchall]
**Learning:** In Python's sqlite3 module, using .fetchall() to materialize intermediate rows into a list before iterating over them is a memory and performance anti-pattern. Direct cursor iteration is highly optimized at the C level.
**Action:** Use direct cursor iteration in a comprehension (e.g., [r[0] for r in cursor]) to avoid intermediate tuple list allocations, significantly reducing peak memory usage and improving execution time.
