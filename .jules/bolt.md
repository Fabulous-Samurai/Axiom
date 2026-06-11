## 2025-06-11 - Optimize SQLite cursor iteration in LargeScaleVarStore
**Learning:** In Python's `sqlite3` module, using `.fetchall()` to materialize intermediate rows into a list before iterating over them (e.g., `[r[0] for r in cursor.fetchall()]`) is a memory and performance anti-pattern. Direct cursor iteration is highly optimized at the C level.
**Action:** Use direct cursor iteration in a comprehension (e.g., `[r[0] for r in cursor]`) to avoid intermediate tuple list allocations, significantly reducing peak memory usage and improving execution time.
