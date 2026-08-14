## 2026-08-14 - [RCE in Python Sandbox]
**Vulnerability:** The `scripts/sandbox.py` script used `eval()` with no environment restrictions, allowing arbitrary code execution (e.g., via `__import__('os')`).
**Learning:** Even within an isolated process, an unrestricted `eval()` poses a significant security risk. A whitelist approach is necessary to safely evaluate mathematical expressions while preventing OS access.
**Prevention:** Always provide a restricted dictionary for `__builtins__` (and globals/locals) when calling `eval()` on user-provided input. Avoid blind `eval()` calls without a safe environment context.
