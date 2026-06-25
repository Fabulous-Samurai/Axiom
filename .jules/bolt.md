## 2025-06-25 - sqlite3 fetchall Memory Anti-pattern
**Learning:** Using `.fetchall()` followed by a list comprehension creates a large intermediate list of row tuples, which is a memory and performance anti-pattern.
**Action:** Always use direct cursor iteration (e.g., `[r[0] for r in cursor]`) to minimize memory allocation and leverage C-level optimizations.
