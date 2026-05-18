## 2025-02-28 - Command Injection in Python Sandbox
**Vulnerability:** `scripts/sandbox.py` used `eval()` to execute user-provided mathematical expressions passed from the command line, enabling Remote Code Execution (RCE) via command injection.
**Learning:** Even when expressions are evaluated in isolated subprocesses, using `eval()` on unsanitized user input is a critical security risk. Attackers can execute arbitrary system commands using built-ins like `__import__('os').system(...)`.
**Prevention:** Always use safe parsing mechanisms instead of `eval()`. For mathematical expressions, `ast.parse` combined with an `ast.NodeVisitor` that strictly allowlists only safe nodes (`Constant`, `BinOp`, `UnaryOp`) and operators effectively mitigates code injection without breaking functionality.
