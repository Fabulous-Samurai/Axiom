## 2024-06-21 - Optimize SQLite Memory Usage in var_store
**Learning:** In Python's sqlite3 module, using `.fetchall()` to materialize intermediate rows into a list before list comprehensions causes unnecessary memory allocation and performance penalties. Direct cursor iteration leverages C-level optimization (`cursor_iternext`).
**Action:** Replace `fetchall()` with direct cursor iteration in `LargeScaleVarStore.keys()` and avoid `.fetchall()` for intermediate lists.
