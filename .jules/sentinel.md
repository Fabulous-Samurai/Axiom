## 2024-05-23 - Command Injection via eval()
**Vulnerability:** Found `eval()` being used in `scripts/sandbox.py` to evaluate arbitrary mathematical expressions passed via strings, allowing potential command injection (e.g., `__import__('os').system(...)`).
**Learning:** Using `eval()` even for seemingly benign mathematical expressions is extremely dangerous when the input is untrusted, as it executes arbitrary Python code. Overly restricting `__builtins__` is often an ineffective anti-pattern.
**Prevention:** Use an AST-based parser (`ast.parse`) combined with a strict `NodeVisitor` that only permits a safelisted set of operations, avoiding `eval()` entirely.
