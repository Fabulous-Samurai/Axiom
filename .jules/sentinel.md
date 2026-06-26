## 2024-06-26 - Unrestricted eval() in Sandbox
**Vulnerability:** The sandbox evaluator in `scripts/sandbox.py` uses unrestricted `eval()`, allowing arbitrary code execution (sandbox escape).
**Learning:** Subprocess isolation (e.g., AppContainer, seccomp) should only act as a secondary defense layer for sandboxes; it is fundamentally unsafe to use Python's `eval()` on untrusted input even within an isolated subprocess.
**Prevention:** To mitigate code injection risks in Python tools without breaking general sandbox functionality, restrict the `eval()` environment (e.g., `eval(expr, {'__builtins__': {}}, {})`).
