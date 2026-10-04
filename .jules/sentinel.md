## 2024-10-04 - [CRITICAL] Prevent Arbitrary Code Execution in Sandbox
**Vulnerability:** The sandbox evaluator `run_isolated_expression` in `scripts/sandbox.py` used `eval()` to execute arbitrary python code, enabling OS access and potential command injection (e.g. `__import__('os').listdir('.')`).
**Learning:** Using Python's `eval()` is insecure even in a sandboxed subprocess since it allows arbitrary module imports and code execution. Disabling builtins is often bypassed using Method Resolution Order (MRO) traversal.
**Prevention:** Always use a strict AST-based evaluator (like `ast.NodeVisitor`) to execute mathematical expressions instead of `eval()`, strictly whitelisting allowed nodes and functions.
