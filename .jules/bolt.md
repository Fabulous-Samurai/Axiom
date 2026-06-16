## 2024-05-24 - [Remove fetchall() for direct SQLite cursor iteration]
**Learning:** Using `.fetchall()` in Python's `sqlite3` materializes intermediate rows into a list before iterating over them. This is a memory and performance anti-pattern. Direct cursor iteration leverages optimized C-level iteration (`cursor_iternext`).
**Action:** Always use direct cursor iteration in list comprehensions instead of `.fetchall()` when dealing with large datasets or bounding memory usage in SQLite queries.
