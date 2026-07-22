## 2026-07-22 - Prevent Sandbox Escape via eval()
**Vulnerability:** The Python `eval()` function was used in `scripts/sandbox.py` without restricting the execution environment, allowing sandbox escapes via Python built-ins like `__import__`.
**Learning:** Using `eval()` on arbitrary input is inherently dangerous, even if executed in a subprocess or a separate execution context, unless `__builtins__` is explicitly restricted.
**Prevention:** Always restrict the execution environment when using `eval()` by providing a safe whitelist dictionary for `__builtins__`. Do not strip it completely (e.g. `{}`) to avoid breaking standard operations.
