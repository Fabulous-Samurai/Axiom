## 2024-10-26 - Command Injection via eval()
**Vulnerability:** Found `eval()` used to execute arbitrary expressions in `scripts/sandbox.py`, leading to RCE (e.g. `__import__('os').listdir('.')`).
**Learning:** Using `eval()` even in restricted subprocesses is unsafe without OS-level sandboxing (AppContainer/seccomp). Restricting `__builtins__` is often an ineffective anti-pattern.
**Prevention:** Use Python's `ast.parse` and a strict `ast.NodeVisitor` (like `safe_eval.py`) to evaluate expressions safely without enabling full code execution.
