## 2026-07-05 - Subprocess eval() Sandbox Escape
**Vulnerability:** The Python `eval()` function in `scripts/sandbox.py` was being used to evaluate expressions in a subprocess without restricting globals/locals, allowing sandbox escape via `__import__` and built-ins.
**Learning:** Even when isolating evaluation inside a subprocess (like `sys.executable -c`), unrestricted `eval()` is vulnerable because it inherits the Python built-in environment, allowing OS access and command execution before any container-level sandbox applies.
**Prevention:** Always restrict `eval()` globals and locals, specifically stripping out `__builtins__` (e.g., `eval(expr, {'__builtins__': {}}, {})`), as a primary defense layer before relying on OS-level isolation.
