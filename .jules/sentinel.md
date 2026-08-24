## 2024-05-24 - Unsafe Python eval() in Sandbox Module
**Vulnerability:** The `run_isolated_expression` function in `scripts/sandbox.py` used `eval()` to execute dynamic Python expressions passed to a subprocess. This allowed command injection, letting attackers evaluate arbitrary code (like `__import__('os').listdir('.')`).
**Learning:** `eval()` should never be used for executing user input, even in a subprocess or restricted context. Modifying `__builtins__` is an ineffective way to secure `eval()`.
**Prevention:** Always use AST-based evaluators with strict NodeVisitors to permit only safe and required operations (math, boolean logic, base data structures).
