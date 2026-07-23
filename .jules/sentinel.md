## 2026-07-23 - Unrestricted eval() in Sandbox
**Vulnerability:** Unrestricted `eval()` in `scripts/sandbox.py` allowed arbitrary code execution and sandbox escapes.
**Learning:** `eval()` should never be used without restricting the `__builtins__` context, as the default context allows access to `__import__` and other sensitive operations.
**Prevention:** Always restrict `eval()` context by explicitly providing a safe whitelist for `__builtins__` and ensure `shell=False` is used in `subprocess.Popen` to prevent command injection.
