## 2024-05-24 - Python `eval()` Command Injection
**Vulnerability:** Command injection via Python's built-in `eval()` function used inside a subprocess string in `scripts/sandbox.py`.
**Learning:** Even within an isolated context or subprocess, directly evaluating user-provided strings using `eval()` allows for arbitrary code execution (e.g., `__import__('os').listdir('.')`).
**Prevention:** Replace `eval()` with a custom AST-based parser (`ast.NodeVisitor`) that restricts execution to a predefined set of safe operations, constants, and functions.
