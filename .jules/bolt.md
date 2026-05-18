## 2024-05-18 - SQLite Cursor Iteration
**Learning:** Using `.fetchall()` materializes a large list of tuples in memory before it is mapped to a list, which is a memory and performance anti-pattern. Direct cursor iteration is highly optimized at the C level (`cursor_iternext`).
**Action:** Replace `fetchall()` with direct cursor iteration: `[r[0] for r in cursor]` to avoid intermediate tuple list allocations.
