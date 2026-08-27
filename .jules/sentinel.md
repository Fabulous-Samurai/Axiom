## 2024-03-24 - Command Injection via eval() in Sandbox
**Vulnerability:** The sandbox evaluator used `eval()` directly inside a subprocess, allowing arbitrary Python execution and OS command injection.
**Learning:** Restricting `__builtins__` is an ineffective anti-pattern for `eval()`. To securely evaluate math/expressions, an AST-based parser must be used.
**Prevention:** Replace all direct `eval()` calls with a strict AST `NodeVisitor` that only permits safe, explicitly defined operations and nodes.
