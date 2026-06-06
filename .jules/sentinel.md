## 2024-05-15 - [Sandbox Code Injection]
**Vulnerability:** Python eval() was used directly on untrusted string input inside scripts/sandbox.py, allowing arbitrary code execution even within the isolated subprocess context.
**Learning:** Subprocess isolation should only act as a secondary defense layer for sandboxes; it is fundamentally unsafe to use Python's eval() on untrusted input even within an isolated subprocess.
**Prevention:** Use an AST-based SafeMathEvaluator whitelist with ast.parse to safely evaluate mathematical expressions instead of eval().
