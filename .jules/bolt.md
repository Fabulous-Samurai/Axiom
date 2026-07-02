## 2024-07-02 - Avoid fetchall() for large sqlite queries
**Learning:** Using `.fetchall()` materializes an intermediate list of tuples in memory, doubling peak memory usage. For large queries, iterating directly on the cursor is more efficient.
**Action:** Use `[r[0] for r in cursor]` instead of `.fetchall()` in SQLite queries.
