## 2025-02-28 - [Sandbox Security] Prevent code injection in sandbox eval

**Vulnerability:** The Python sandbox tool (`scripts/sandbox.py`) used `eval()` to execute user-provided expressions. This allows arbitrary code execution and sandbox escape (e.g., via `__import__('os').listdir('.')`).
**Learning:** `eval()` should never be used on untrusted input, even if executed in a subprocess. Subprocesses alone do not provide sufficient isolation without OS-level restrictions like seccomp or AppContainer.
**Prevention:** Use `ast.parse` with `ast.NodeVisitor` to safely evaluate mathematical expressions by strictly whitelisting allowed AST node types and operations (e.g., `ast.Add`, `ast.Constant`) while explicitly denying dangerous nodes like `ast.Call` and `ast.Attribute`.
