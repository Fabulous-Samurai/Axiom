## 2024-05-24 - Avoid `.fetchall()` on SQLite queries
**Learning:** Using `.fetchall()` to materialize intermediate rows into a list before iterating over them (e.g., `[r[0] for r in cursor.fetchall()]`) is a memory and performance anti-pattern.
**Action:** Use direct cursor iteration in a comprehension (e.g., `[r[0] for r in cursor]`) to avoid intermediate tuple list allocations, significantly reducing peak memory usage and improving execution time.
