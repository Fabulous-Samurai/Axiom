## 2024-05-18 - [Python AST Safe Evaluation Strategy]
**Vulnerability:** Found arbitrary code execution vulnerability via the built-in `eval()` in `scripts/sandbox.py`.
**Learning:** Python's `eval()` is inherently unsafe because it allows referencing builtin functions and imports (e.g., using `__import__` or MRO traversal) unless strictly constrained. Disabling `__builtins__` in `eval` is bypassable.
**Prevention:** Always use `ast.parse` and an `ast.NodeVisitor` approach to whitelist specific safe nodes instead of using `eval()`.
