## $(date +%Y-%m-%d) - Command Injection in sandbox.py
**Vulnerability:** Arbitrary code execution via `eval()` in `scripts/sandbox.py` allowing command injection.
**Learning:** `eval()` was used to parse expressions directly, leading to command injection risks. It wasn't restricted securely.
**Prevention:** Use a safe AST-based evaluator (`ast.NodeVisitor`) to whitelist safe mathematical operations instead of using `eval()`.
