## 2024-05-18 - Unrestricted eval() in Sandbox
**Vulnerability:** The `scripts/sandbox.py` utility used an unrestricted `eval()` call within a subprocess, allowing arbitrary Python code execution (like `__import__('os')`) despite process isolation.
**Learning:** Process isolation (like AppContainer or seccomp) is only a secondary defense. Python's `eval()` must be secured at the language level by stripping built-ins when parsing untrusted expressions.
**Prevention:** Always restrict the environment globals and locals, explicitly stripping out `__builtins__` (e.g., `eval(expr, {'__builtins__': {}}, {})`) to prevent arbitrary code execution while retaining general expression parsing.
