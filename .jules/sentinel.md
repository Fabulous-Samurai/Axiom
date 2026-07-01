## 2024-07-01 - [Arbitrary Code Execution in Sandbox via Unfiltered Eval]
**Vulnerability:** Arbitrary Python code execution was possible in `scripts/sandbox.py` because `eval()` was called without restricting its environment.
**Learning:** Subprocess isolation is insufficient if the subprocess itself executes untrusted input via `eval()`. Without explicitly clearing `__builtins__`, malicious input could import modules and access system resources.
**Prevention:** Always restrict the globals and locals dictionaries when using `eval()` (e.g., `eval(expr, {'__builtins__': {}}, {})`) to prevent sandbox escapes, or use AST-based evaluators when general expression parsing isn't required.
