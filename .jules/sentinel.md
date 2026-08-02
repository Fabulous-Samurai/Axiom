## 2026-08-02 - AST Evaluation over restricted eval()
**Vulnerability:** Arbitrary code execution vulnerability via PyJail bypass in sandbox subprocess.
**Learning:** Restricting `__builtins__` in Python's `eval()` is security theater. Attackers can bypass it using class introspection (e.g., `().__class__.__bases__[0].__subclasses__()`) to access standard libraries like `os`.
**Prevention:** Never use `eval()` for untrusted input. Always use a strict AST-based literal and math evaluator (like `ast.parse`) that explicitly whitelists safe nodes, operators, and functions.
