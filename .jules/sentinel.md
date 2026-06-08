## 2024-06-08 - Python Eval in Sandbox Subprocess
**Vulnerability:** Code injection vulnerability caused by using Python's `eval()` on untrusted input inside a subprocess intended as a sandbox.
**Learning:** Subprocess isolation (like AppContainer or seccomp) should only act as a secondary defense layer; it is fundamentally unsafe to use Python's `eval()` on untrusted input even within an isolated subprocess, as it permits arbitrary code execution.
**Prevention:** Use `ast.parse` with a whitelist of safe nodes (like `ast.Constant`, `ast.BinOp`) implemented via an `ast.NodeVisitor` instead of `eval()` for evaluating dynamic mathematical expressions.
