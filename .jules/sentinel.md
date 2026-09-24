## YYYY-MM-DD - Prevent MRO Traversal in Python Sandbox
**Vulnerability:** Arbitrary code execution via Python's `eval()`.
**Learning:** Restricting `__builtins__` in `eval()` is insufficient because attackers can use Method Resolution Order (MRO) traversal (e.g., `().__class__.__bases__[0].__subclasses__()`) to bypass globals restrictions.
**Prevention:** Filter out dangerous substrings (e.g., `__` and `import`) from the evaluated expression and execute using a strictly whitelisted `__builtins__` dictionary.
