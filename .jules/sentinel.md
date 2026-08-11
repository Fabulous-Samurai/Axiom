## 2026-08-11 - Fix Code Injection in Python Sandbox
**Vulnerability:** Code injection vulnerability in `scripts/sandbox.py` where user-provided expressions are evaluated via `eval()` without any scope restrictions, allowing arbitrary Python code execution (e.g., `__import__('os').system(...)`).
**Learning:** `eval()` allows full access to the Python environment by default if a restricted environment is not explicitly provided. Sandboxing by running `eval()` in a separate process isn't enough to prevent access to the environment/file system of the subprocess itself.
**Prevention:** Always restrict the `eval()` environment with a safe `__builtins__` whitelist when evaluating general Python expressions, instead of allowing access to all builtins like `__import__`.
