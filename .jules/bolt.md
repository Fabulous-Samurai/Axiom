## 2024-07-01 - Avoid fetchall() Memory Spikes
**Learning:** In python, iterating over sqlite3 query results with `.fetchall()` materializes intermediate rows into a list, allocating memory and increasing peak memory usage. Using direct cursor iteration (e.g. `[r[0] for r in cursor]`) avoids this overhead.
**Action:** Replace `fetchall()` with cursor iteration when working with large or potentially large SQLite results.
