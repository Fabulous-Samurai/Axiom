## 2026-08-09 - Prevent Sandbox Code Injection via Unrestricted eval()
**Vulnerability:** The sandbox evaluator `scripts/sandbox.py` allowed arbitrary Python code execution because it passed user expressions to an unrestricted `eval()` in a subprocess, leading to potential RCE/command injection.
**Learning:** Evaluating general expressions dynamically using `eval()` without restricting `__builtins__` exposes the system to code injection. The system requirement to support general expressions prevented the use of safe AST parsing, forcing reliance on `eval()`.
**Prevention:** Always restrict the `__builtins__` context (e.g., passing a whitelist dictionary like `{'__builtins__': {'abs': abs}}` to `eval()`) when dynamically evaluating expressions that cannot be parsed by strict AST means.
