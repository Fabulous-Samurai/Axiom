## 2024-05-24 - [Fix arbitrary code execution in sandbox via eval]
**Vulnerability:** The Python `eval()` function was being used on untrusted input inside a subprocess sandbox, which permitted arbitrary code execution (e.g., `__import__('os')`).
**Learning:** Subprocess isolation (like seccomp or AppContainer) is an important defense-in-depth layer, but it is fundamentally unsafe to use `eval()` on untrusted input even within isolated environments.
**Prevention:** To safely evaluate mathematical expressions, use an AST-based parser (`ast.parse`) with a strict whitelist of allowed nodes (like `ast.Constant` and arithmetic operators) instead of using `eval()`.
