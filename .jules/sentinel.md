## 2026-07-08 - Secure eval() environment in subprocess
**Vulnerability:** Arbitrary code execution via unrestricted Python `eval()` in `scripts/sandbox.py`.
**Learning:** Even when expressions are isolated in a subprocess, unrestricted `eval()` allows dangerous imports (like `os` or `subprocess`) and built-ins. String formatting with `%` must be used instead of f-strings or `.format()` when injecting code with curly braces to avoid `KeyError` syntax errors.
**Prevention:** Always restrict `eval()` globals and locals (e.g., `eval(expr, {'__builtins__': {}}, {})`) when evaluating untrusted mathematical expressions, even within an isolated subprocess layer.
