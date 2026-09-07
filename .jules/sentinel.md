## 2025-02-28 - Replace raw eval() with AST-based evaluation in sandbox
**Vulnerability:** The sandbox subprocess used raw `eval()` to execute expressions provided via sys.argv or from the engine, allowing arbitrary code execution (e.g. `__import__('os').listdir('.')`).
**Learning:** Evaluating untrusted expressions with Python's built-in `eval()` is critically dangerous. It's often used for convenience but lacks sufficient security boundaries.
**Prevention:** Use an AST-based parser (`ast.NodeVisitor`) to safely evaluate strictly whitelisted nodes (like math operations and basic literals), completely forbidding dangerous nodes like imports, loops, and private attributes.
