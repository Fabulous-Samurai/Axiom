## YYYY-MM-DD - Arbitrary Code Execution in Sandbox via eval()
**Vulnerability:** The isolated sandbox (`scripts/sandbox.py`) used Python's `eval()` function to parse mathematical expressions inside a subprocess, allowing arbitrary code execution (e.g., `__import__('os').listdir('.')`).
**Learning:** Even within an isolated subprocess, `eval()` remains highly dangerous for untrusted input, as standard mitigations (like wiping `__builtins__`) can be bypassed using MRO traversal.
**Prevention:** For processing mathematical expressions securely, replace `eval()` with a strict AST-based evaluator (`ast.NodeVisitor`) that only implements safe operations (e.g., arithmetic operators and standard math library functions).
