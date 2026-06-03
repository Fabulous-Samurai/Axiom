## 2024-05-15 - Optimize SQLite fetchall
**Learning:** In Python's sqlite3 module, using .fetchall() to materialize intermediate rows into a list before iterating over them is a memory and performance anti-pattern.
**Action:** Use direct cursor iteration in a comprehension to avoid intermediate tuple list allocations, significantly reducing peak memory usage and improving execution time.
