
## 2024-05-18 - Dictionary Iteration Tuple Allocation
**Learning:** In Python, iterating over a dictionary using `for key in tuple(d.keys()):` creates an unnecessary tuple allocation. When iterating over a dictionary `d` while mutating another dictionary `d2`, iterating directly with `for key in d:` is perfectly safe and avoids the allocation, which is critical on hot paths like cache evictions.
**Action:** Use direct dictionary iteration `for key in d:` instead of `tuple(d.keys())` when the dictionary being iterated over is not being mutated during the iteration.
