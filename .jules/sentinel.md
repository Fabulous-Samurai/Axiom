## 2024-05-18 - [Fix Python eval() vulnerability in Visualization Tool]
**Vulnerability:** Use of `eval()` in `tools/visualization/advanced_3d_visualization.py` allowing Remote Code Execution (RCE) / Code Injection even with a restricted `__builtins__` context.
**Learning:** Python's `eval()` is never truly safe if `__builtins__` is just empty because malicious users can still break out of the sandbox using object introspection mechanisms like `().__class__.__base__.__subclasses__()` to access dangerous modules like `os`.
**Prevention:** Use `ast.parse` and recursively evaluate nodes within a strict whitelist of safe AST node types (like `ast.Constant`, `ast.BinOp`, `ast.Call` on explicit allowed function names). Always explicitly reject `ast.Attribute` to prevent introspection attacks.
