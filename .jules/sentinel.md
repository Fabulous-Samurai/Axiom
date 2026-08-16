## 2026-08-16 - Prevent Arbitrary Code Execution in Python sandboxes using Restricted eval()
**Vulnerability:** The Python sandbox evaluation (scripts/sandbox.py) used an unrestricted eval() allowing command injection/arbitrary code execution (e.g. `__import__('os').listdir('.')`).
**Learning:** Evaluating general mathematical expressions in Python via `eval()` must not allow access to built-in functions that can perform malicious acts. Using an AST-based parser might break complex test expressions.
**Prevention:** Always restrict the `eval()` execution context using a safe `__builtins__` whitelist mapping (e.g. `{'__builtins__': {'abs': abs, 'min': min, ...}}`) to prevent access to standard built-in functions like `__import__` while retaining mathematical functionalities.
