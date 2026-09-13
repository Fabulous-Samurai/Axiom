## 2023-10-27 - Command Execution via eval
**Vulnerability:** Command execution via `eval()` in `scripts/sandbox.py`
**Learning:** Disabling builtins is not enough; explicit AST-based whitelisting is needed to prevent MRO traversal and other bypasses.
**Prevention:** Use `ast.parse` and recursive whitelisting of expected nodes.
