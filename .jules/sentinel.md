## 2026-10-16 - Secure Python Expression Evaluation
**Vulnerability:** Python's built-in `eval()` was used directly in `scripts/sandbox.py`, allowing Remote Code Execution (RCE) via `__import__` and `__class__` traversals.
**Learning:** Using `eval` even with limited globals (`{"__builtins__": {}}`) is fundamentally insecure in Python due to introspection capabilities (like `().__class__.__bases__[0].__subclasses__()`).
**Prevention:** Always use a custom AST (Abstract Syntax Tree) evaluator (`ast.NodeVisitor`) with explicit allowlists for operators, functions, and nodes to strictly sandbox mathematical/expression evaluation.
