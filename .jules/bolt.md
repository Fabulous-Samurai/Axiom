## 2024-05-24 - Optimize SQLite cursor iteration
**Learning:** In Python's sqlite3 module, direct cursor iteration (`for row in cursor`) is highly optimized in C and avoids the large intermediate list allocation overhead of `.fetchall()`.
**Action:** Use direct cursor iteration (`[r[0] for r in cursor]`) instead of `.fetchall()` followed by a loop or comprehension when materializing query results.
