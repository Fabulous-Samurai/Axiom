## 2025-02-14 - Replace eval() with ast.parse in scripts/sandbox.py
**Vulnerability:** The `scripts/sandbox.py` script was using Python's built-in `eval()` to execute user-provided code within a subprocess. This created an arbitrary code execution vulnerability.
**Learning:** Even if the sandbox executes code within a subprocess (e.g. using AppContainer or seccomp), it's fundamentally unsafe to use `eval()` on untrusted input, because it allows users to break out of process isolation if the isolation is weak or run arbitrary Python code with the permissions of the parent process.
**Prevention:** Always use safe abstract syntax tree evaluation, like `ast.parse` in `eval` mode with an explicitly allowed node whitelist (e.g. `SafeMathEvaluator`), instead of using Python's native `eval()`.
