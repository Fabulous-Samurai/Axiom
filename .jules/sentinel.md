## 2024-05-18 - Sandbox Arbitrary Code Execution Fixed
**Vulnerability:** Found arbitrary code execution vulnerability via unsafe eval() usage in scripts/sandbox.py
**Learning:** Even if the process was limited using OS isolation, using Python's raw eval() inside it allows any valid python to run, bypassing application intent. Built-in disabling via `{"__builtins__": {}}` is insufficient due to Method Resolution Order (MRO) traversal escapes (e.g. `().__class__.__bases__[0].__subclasses__()`).
**Prevention:** Always use an AST-based whitelist approach (ast.NodeVisitor) to restrict parsing to only expected structures, instead of blacklisting or trusting OS-level isolation alone.
