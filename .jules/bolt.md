## 2024-07-28 - Avoid fetching all rows into memory at once in var_store
**Learning:** Using `.fetchall()` on a sqlite3 cursor with a large `LIMIT` loads all rows into a list of tuples, which is inefficient. Iterating the cursor directly avoids this overhead.
**Action:** Replace `.fetchall()` followed by a list comprehension with a list comprehension that iterates directly on the cursor.
