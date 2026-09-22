## 2024-05-15 - Replace Python eval() with Safe AST Evaluator
**Vulnerability:** The sandbox evaluation logic used `eval()` with only basic string replacement for safety, which left it vulnerable to arbitrary code execution (e.g. `__import__('os').listdir('.')`).
**Learning:** Even simple expression evaluators in "sandbox" mode can be bypassed if `eval()` is used, as attributes and built-ins can be accessed dynamically.
**Prevention:** Always use `ast.parse` and an `ast.NodeVisitor` or strict whitelisting of functions/operators to parse and compute untrusted expressions. Never rely on `eval()` for safety.
