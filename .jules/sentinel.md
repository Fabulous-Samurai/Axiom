## 2024-05-15 - Unsafe Python eval in isolation scripts
**Vulnerability:** Arbitrary code execution vulnerability in `scripts/sandbox.py` due to using Python's `eval()` function to evaluate expressions passed as strings, allowing access to builtins and OS functions.
**Learning:** `eval()` should never be used to evaluate untrusted input, even when running in a supposedly "isolated" subprocess or when attempting to override `__builtins__`. Attackers can use Method Resolution Order (MRO) traversal (e.g., `().__class__.__bases__[0].__subclasses__()`) to recover references to modules like `os` or `sys`.
**Prevention:** Use a strict AST-based whitelisting approach (e.g., `ast.NodeVisitor`) to safely evaluate math expressions, completely removing the reliance on `eval()`.
