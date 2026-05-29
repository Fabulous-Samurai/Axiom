## 2024-05-18 - [CRITICAL] Fix command injection vulnerability in sandbox
**Vulnerability:** Sandbox used unsafe Python `eval()` which allows arbitrary command execution.
**Learning:** Using `eval` on user input even with restricted builtins can be easily bypassed to achieve command execution (e.g., via `__import__('os')`).
**Prevention:** Use an AST parser with explicit whitelisting of safe nodes (e.g. `ast.parse` and a custom `ast.NodeVisitor` like `SafeMathEvaluator`) instead of `eval()`.
