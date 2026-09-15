## 2025-03-08 - [Sandbox Escape via MRO Traversal]
**Vulnerability:** The Python `eval()` function was used to safely evaluate mathematical expressions by passing `{"__builtins__": {}}`. However, this restriction can be bypassed via Method Resolution Order (MRO) traversal (e.g. `().__class__.__bases__[0].__subclasses__()`), which recovers access to builtin modules like `os` and `sys`, leading to arbitrary code execution (ACE).
**Learning:** Emptying `__builtins__` in `eval()` does NOT provide a secure sandbox in Python due to reflection capabilities that allow reaching the global namespace.
**Prevention:** Never use `eval()` for untrusted input. Use `ast.literal_eval` for literals or implement a strict `ast.NodeVisitor` approach to whitelist only specific, safe AST nodes and operations.
