## 2024-05-31 - Sandbox Evaluation Vulnerability
**Vulnerability:** Arbitrary code execution (Command Injection / Sandbox Escape) via unsafe use of `eval()` in `scripts/sandbox.py`.
**Learning:** `eval()` in Python was used to evaluate untrusted input expressions, which allowed trivial bypassing of the complexity guard and remote code execution (e.g. `__import__('os').listdir('.')`).
**Prevention:** Always use safe parsing libraries like `ast.literal_eval` for constants, or implement a custom, strict `ast.NodeVisitor` to securely evaluate algebraic expressions without allowing arbitrary function calls or OS module imports.
