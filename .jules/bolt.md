## 2024-08-02 - SQLite Fetchall Optimization
**Learning:** Using `.fetchall()` on a sqlite3 cursor materializes intermediate rows into a list, increasing peak memory usage. Direct cursor iteration is more efficient, as per guidelines.
**Action:** Replace `rows = cursor.fetchall(); return [r[0] for r in rows]` with `return [r[0] for r in cursor]` to avoid unnecessary intermediate tuple list allocations.
