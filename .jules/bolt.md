## 2026-07-04 - Avoid .fetchall() in sqlite3 for large result sets
**Learning:** Using .fetchall() creates an intermediate list of tuples which spikes memory usage, especially for large UI caps like 100k rows.
**Action:** Use direct cursor iteration [r[0] for r in cursor] to avoid intermediate tuple list allocations and reduce peak memory footprint.
