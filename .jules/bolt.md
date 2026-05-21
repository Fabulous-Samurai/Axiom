## 2024-05-14 - [SQLite fetchall Memory Optimization]
**Learning:** In Python's sqlite3 module, using .fetchall() to materialize intermediate rows into a list before iterating over them (e.g., `[r[0] for r in cursor.fetchall()]`) is a memory and performance anti-pattern. Direct cursor iteration in a comprehension (e.g., `[r[0] for r in cursor]`) avoids intermediate tuple list allocations, significantly reducing peak memory usage and improving execution time.
**Action:** Replace `fetchall()` with direct cursor iteration where appropriate to save memory and execution time.
