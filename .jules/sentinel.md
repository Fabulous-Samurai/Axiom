## 2024-05-20 - Unsafe `eval()` allows sandbox escape via MRO traversal
**Vulnerability:** The Python expression sandbox in `scripts/sandbox.py` relied on direct `eval()` without restricting `__builtins__` or limiting operations.
**Learning:** Even if `__builtins__` are set to `{}`, attackers can still bypass the sandbox using Method Resolution Order (MRO) traversal (e.g., `().__class__.__bases__[0].__subclasses__()`) to access dangerous modules like `os`.
**Prevention:** Use an AST-based parser (`ast.parse`) to thoroughly inspect and whitelist structural nodes before evaluating expressions, preventing arbitrary code execution and imports entirely.
