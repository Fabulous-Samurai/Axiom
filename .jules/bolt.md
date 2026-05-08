## 2024-05-24 - Optimize LargeScaleVarStore.all_keys with fetchmany
**Learning:** In Python SQLite wrappers, iterating over a query result row-by-row (e.g., `for row in cursor:`) causes frequent crossing of the Python/C boundary, which creates measurable overhead when processing large datasets (millions of records). Batching row retrieval using `cursor.fetchmany(N)` significantly reduces this overhead.
**Action:** When extracting large amounts of data from a SQLite database into Python memory, use `cursor.fetchmany()` (e.g. `1000`) instead of row-by-row iteration.
