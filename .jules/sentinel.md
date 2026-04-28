## 2024-05-01 - Prevent arbitrary code execution in Sandbox
**Vulnerability:** The Python Sandbox tool `scripts/sandbox.py` previously evaluated AXIOM expressions using the built-in `eval()` function, which allowed for arbitrary code execution and Sandbox escapes (e.g. `__import__('os').listdir('.')`).
**Learning:** `eval()` should never be used for evaluating mathematical or logical expressions from potentially untrusted input, even if the environment appears isolated, as it easily leads to code injection.
**Prevention:** Implement a strict AST-based evaluator (`ast.NodeVisitor`) with a whitelist of safe operations, specifically blocking `ast.Attribute` and `ast.Call` nodes to prevent function calls and object traversal.
