## 2024-06-15 - [SafeMathEvaluator instead of eval in Sandbox]
**Vulnerability:** Arbitrary Code Execution via `eval()`.
**Learning:** Subprocess isolation (e.g., AppContainer, seccomp) should only act as a secondary defense layer for sandboxes; it is fundamentally unsafe to use Python's `eval()` on untrusted input even within an isolated subprocess.
**Prevention:** Use an AST-based parser (`SafeMathEvaluator` using `ast.parse` and whitelist nodes) instead of `eval()` to guarantee that only expected mathematical expressions can be interpreted.
