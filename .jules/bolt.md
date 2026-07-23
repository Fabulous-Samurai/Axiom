## 2026-07-23 - SQLite Cursor Iteration
**Learning:** Using `.fetchall()` materializes intermediate tuple lists which causes unnecessary memory overhead.
**Action:** Use direct cursor iteration (e.g. `[r[0] for r in cursor]`) to avoid allocating intermediate lists for large result sets.
