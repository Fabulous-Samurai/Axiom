## 2026-08-04 - Prevent Arbitrary Code Execution in Sandbox
**Vulnerability:** Arbitrary code execution vulnerability in `scripts/sandbox.py` because `eval()` was used without a restricted environment.
**Learning:** Using `eval()` even in a subprocess allows arbitrary python code execution.
**Prevention:** Always use a restricted environment for `eval()` by passing a safe `__builtins__` whitelist.
