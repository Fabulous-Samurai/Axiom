## 2024-06-10 - Sandbox eval() arbitrary code execution
**Vulnerability:** Arbitrary code execution via Python's built-in `eval()` function in `scripts/sandbox.py`.
**Learning:** Even when `eval()` is isolated in a subprocess with time and memory limits, it can still execute arbitrary code (e.g., `__import__('os').listdir('.')`) which bypasses the intended constraints of a purely mathematical expression evaluator. Subprocess isolation acts only as a secondary defense layer; relying on `eval()` is fundamentally unsafe.
**Prevention:** Use an AST-based parser (`ast.parse`) combined with a `NodeVisitor` class that whitelists only safe, required operations (such as `ast.Add`, `ast.Sub`, `ast.Constant`) to evaluate expressions without exposing a direct execution environment.
