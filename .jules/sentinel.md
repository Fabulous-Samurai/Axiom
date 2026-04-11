
## 2024-05-18 - [CRITICAL] Code Injection in Python 3D Visualization Script via `eval()`
**Vulnerability:** The Python script `tools/visualization/advanced_3d_visualization.py` used the built-in `eval()` function to parse user-provided mathematical functions and strings. Even with `{"__builtins__": {}}` provided as globals, `eval` could be exploited using object introspection (e.g., `().__class__.__bases__[0].__subclasses__()`) allowing command injection and sandbox escape.
**Learning:** Python's built-in `eval` is never safe when dealing with user inputs, regardless of what dictionary is passed to `globals` or `locals`. The object hierarchy can always be accessed.
**Prevention:** A custom, AST-based safe math evaluator must be used that restricts parsing to an explicit whitelist of safe syntax tree nodes (like ast.BinOp, ast.Call) while explicitly rejecting ast.Attribute to prevent namespace traversal and code execution.
