## 2026-07-26 - Memory Overhead of SQLite fetchall()
**Learning:** Using `.fetchall()` on a SQLite cursor materializes all rows into an intermediate list of tuples before they can be unpacked or processed. This creates a temporary memory spike and allocation overhead, especially noticeable when dealing with large numbers of rows.
**Action:** Iterate directly over the cursor (e.g., `[r[0] for r in cursor]`) instead of using `.fetchall()` to avoid the intermediate list allocation, saving memory and improving performance.
