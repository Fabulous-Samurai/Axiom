## 2025-02-24 - [Fix Python eval() vulnerability]
**Vulnerability:** Arbitrary code execution via `eval()` in `scripts/sandbox.py`.
**Learning:** Even when expressions are passed to a restricted subprocess, using Python's raw `eval()` enables trivial sandbox escapes via built-in functions like `__import__('os')`.
**Prevention:** Always implement a custom `ast.NodeVisitor` that strictly allowlists AST nodes and explicitly checks attribute access to prevent evaluating untrusted operations or names.
