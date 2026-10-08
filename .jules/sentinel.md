## 2024-10-08 - Secure Python evaluation with AST whitelisting
**Vulnerability:** Arbitrary Code Execution (ACE) via `eval()` in `scripts/sandbox.py`.
**Learning:** Using `eval()` even with restricted `__builtins__` can be bypassed using Method Resolution Order (MRO) or accessing special attributes like `__import__` directly if the expression string allows it.
**Prevention:** Use a strict Abstract Syntax Tree (AST) node visitor (`ast.NodeVisitor`) to whitelist allowed functions, variables, and operations instead of trying to blacklist dangerous ones. Enforcing type checks on mathematical operations (e.g. `ast.Mult` and `ast.Pow`) is also necessary to prevent resource exhaustion via sequence repetition (e.g. `[0] * 10**9`).
