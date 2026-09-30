## 2024-05-14 - Replace eval() with Safe AST Evaluation
**Vulnerability:** Arbitrary code execution via Python's built-in `eval()` function in `scripts/sandbox.py`.
**Learning:** `eval()` can be bypassed even with `__builtins__` disabled by using MRO traversal (e.g. `().__class__.__bases__[0].__subclasses__()`). This allows escaping the sandbox environment to access modules like `os` or `sys`.
**Prevention:** Always use safe parsing techniques for executing dynamic user expressions, such as AST whitelist evaluation, rather than raw `eval()`.
