## 2024-03-01 - Arbitrary Code Execution in Sandbox via eval()
**Vulnerability:** The `scripts/sandbox.py` file evaluated user-provided strings using Python's built-in `eval()` function inside a subprocess (`eval(expression)`). This allowed arbitrary Python code execution (e.g. `__import__('os')`).
**Learning:** Subprocess isolation alone is insufficient if the subprocess evaluates unsanitized inputs using `eval()`, which allows full Python runtime access.
**Prevention:** Avoid `eval()` for untrusted input. Use `ast.parse` to generate an Abstract Syntax Tree (AST) and enforce a strict whitelist of allowed functions and operators via a custom `ast.NodeVisitor`.
