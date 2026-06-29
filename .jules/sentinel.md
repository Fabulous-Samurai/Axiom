## 2024-05-24 - Sandbox RCE via Unrestricted Eval
**Vulnerability:** The Python `eval()` function was being used in `scripts/sandbox.py` without restricting the execution environment, allowing arbitrary code execution (e.g., via `__import__`).
**Learning:** Process isolation alone is insufficient if the subprocess allows arbitrary code to run with the permissions of the user. Python's `eval()` defaults to inheriting the global environment if not constrained.
**Prevention:** Always restrict the `eval()` environment globals and locals explicitly, particularly stripping out `__builtins__` (e.g., `eval(expr, {'__builtins__': {}}, {})`), to prevent code execution while retaining safe mathematical expression parsing capability.
