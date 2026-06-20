## 2024-06-20 - Sandbox eval Vulnerability
**Vulnerability:** Arbitrary code execution via unfiltered eval in `scripts/sandbox.py`
**Learning:** Python's `eval()` function can execute arbitrary code if not restricted. Subprocess isolation acts as a secondary layer but it is fundamentally unsafe to use unfiltered eval on untrusted input. Overriding `__builtins__` inside the `eval()` call prevents access to standard Python functions like `__import__`.
**Prevention:** Always restrict the `eval()` environment globals and locals, explicitly stripping out `__builtins__` (e.g., `eval(expr, {'__builtins__': {}}, {})`).
