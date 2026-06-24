## 2024-06-24 - Optimize SQLite3 fetchall() memory allocation
**Learning:** Using `cursor.fetchall()` before a list comprehension creates a massive intermediate list of tuples in memory, which is a performance and memory anti-pattern.
**Action:** Always iterate directly over the SQLite cursor (e.g., `[r[0] for r in cursor]`) which is highly optimized at the C level and avoids secondary list allocations.
