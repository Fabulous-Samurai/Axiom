## 2026-07-16 - Securing Python `eval` in subprocess

**Vulnerability:** The `eval` function was used without an environment restriction (specifically the `__builtins__` dictionary), which allowed command and code injection vulnerabilities via `__import__('os')`.
**Learning:** Python `eval` uses standard global builtins if not overridden, allowing sandbox escapes even inside subprocesses, bypassing simple isolation.
**Prevention:** Always restrict the `eval` environment globals and locals, especially by replacing `__builtins__` with a safe whitelist (e.g. `{'__builtins__': {'abs': abs, 'min': min, 'max': max, 'int': int, 'float': float}}`).
