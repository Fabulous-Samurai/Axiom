## 2024-05-18 - Sandbox Command Injection via Python Eval
**Vulnerability:** Python `eval()` in `scripts/sandbox.py` allowed arbitrary code execution and sandbox escape.
**Learning:** Relying on subprocesses with generic Python `eval()` is critically insecure. Disabling builtins (`{"__builtins__": {}}`) is insufficient due to MRO class hierarchy traversal (e.g., `().__class__.__bases__[0].__subclasses__()`), which can recover references to modules like `os` or `sys`.
**Prevention:** Implement a strict AST-based `NodeVisitor` (e.g., `safe_eval.py`) that explicitly whitelists safe nodes, operators, and functions instead of relying on runtime `eval()`.
