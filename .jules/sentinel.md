## 2023-10-25 - [Sandbox Escape via eval()]
**Vulnerability:** Command injection / Sandbox Escape via `eval()` in `scripts/sandbox.py`.
**Learning:** Using `eval()` to execute expressions even in isolated subprocesses is risky, because Python's builtins provide extensive system access (e.g., `__import__`).
**Prevention:** Use `ast.parse()` combined with a restrictive `ast.NodeVisitor` that only supports explicitly whitelisted operations, completely avoiding code execution.
