## 2024-05-15 - Strict AST Evaluator for Python Sandbox
**Vulnerability:** Arbitrary code execution via `eval()` in `scripts/sandbox.py`. Disabling `__builtins__` is insufficient because Method Resolution Order (MRO) traversal allows retrieving `os` and other builtins.
**Learning:** For a mathematical sandbox, `eval()` shouldn't be used at all. Even if `__builtins__` is restricted, `__import__` can sometimes be found. The only secure way is an AST visitor that whitelists only specific operators and math functions.
**Prevention:** Use a strict AST evaluator (`ast.NodeVisitor`) that parses the expression with `ast.parse` and manually evaluates known safe operations, completely bypassing Python's `eval` execution engine.
