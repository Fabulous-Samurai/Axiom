## 2024-04-27 - Optimize ResultCache eviction allocation
**Learning:** In the GUI helper's `ResultCache`, dict keys were being converted to a tuple (`tuple(old["entries"].keys())`) during block eviction. This was unnecessary since the loop only reads `old["entries"]` to mutate `self._key_index`.
**Action:** When iterating over a dictionary merely to read its keys while mutating a *different* dictionary, use direct dictionary iteration (`for key in dict:`) instead of forcing an O(N) allocation with `tuple(dict.keys())` or `list(dict.keys())`.
