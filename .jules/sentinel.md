## 2024-05-24 - Securing Python Evaluation
**Vulnerability:** Use of `eval()` allowed arbitrary code execution and sandbox escapes because it didn't prevent Method Resolution Order (MRO) traversal or loading modules like `os` or `sys`.
**Learning:** Even if `eval()` is isolated in a subprocess, attackers can escape by traversing builtins or using MRO (e.g. `().__class__.__bases__[0]...`). Disabling `__builtins__` alone is insufficient if dangerous substrings aren't blocked.
**Prevention:** Fast-fail obviously dangerous keywords like `__`, `import`, `eval`, `exec` before using `eval()` with disabled built-ins, or prefer a strict AST-based whitelisting approach (e.g. `ast.NodeVisitor`).
