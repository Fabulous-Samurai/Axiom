# Bolt's Performance Journal
## 2024-05-24 - ResultCache dictionary iteration optimization
**Learning:** Using `tuple(d.keys())` when iterating over a dictionary for cache eviction causes unnecessary tuple allocations and degrades performance. Direct iteration (`for key in d:`) is significantly more efficient when not modifying the dictionary's keys during the loop.
**Action:** Always prefer direct dictionary iteration `for key in d:` over creating intermediate collections like tuples or lists of keys unless the dictionary structure is being modified during iteration.
