## 2026-07-08 - Avoid .fetchall() on large SQLite queries
**Learning:** Using .fetchall() materializes a large intermediate list of tuples in memory, which significantly increases peak memory usage for large views.
**Action:** Use direct cursor iteration (e.g., [r[0] for r in cursor]) to avoid intermediate allocations when dealing with large datasets.
