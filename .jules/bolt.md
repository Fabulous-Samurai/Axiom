## 2024-05-11 - [Optimize LargeScaleVarStore.all_keys iterator]
**Learning:** For SQLite large datasets in Python, single row iteration via `for row in cursor:` is significantly slower than batching fetches using `cursor.fetchmany(N)` due to Python/C boundary crossing overhead for every single row.
**Action:** Use `cursor.fetchmany(1000)` inside a while loop when iterating over potentially millions of rows instead of iterating directly on the SQLite cursor.
