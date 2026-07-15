## 2026-07-15 - Securing eval() with restricted builtins
**Vulnerability:** Arbitrary code execution (Command Injection) via unfiltered `eval()` in `scripts/sandbox.py` expression sandbox.
**Learning:** Subprocess isolation (running code via `python -c`) is insufficient for a sandbox if `eval()` has access to standard `__builtins__`, as an attacker can still use `__import__` to compromise the host system.
**Prevention:** Always restrict `__builtins__` in `eval()` or use `ast.literal_eval`. For a math expression evaluator, supply a safe whitelist (e.g., `{'abs': abs, ...}`) in the global context dictionary.
