## 2025-05-18 - Fix arbitrary code execution in Sandbox Evaluator
**Vulnerability:** Arbitrary code execution via Python's `eval()` in `scripts/sandbox.py`.
**Learning:** Using `eval(expr)` directly even in a subprocess is inherently unsafe as it allows sandbox escapes like `__import__('os').system('...')`.
**Prevention:** Always implement a custom `ast.NodeVisitor` approach and strictly whitelist allowed operations to execute untrusted code.
