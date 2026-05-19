## 2025-05-19 - [Arbitrary Code Execution in Sandbox]
**Vulnerability:** The sandbox evaluator `scripts/sandbox.py` was passing unsanitized expressions to Python's `eval()` function, resulting in arbitrary code execution (e.g. `__import__('os').system('...')`).
**Learning:** Naively using `eval()` for untrusted input, even inside a subprocess, is a CRITICAL RCE vector since typical Python environments will interpret arbitrary malicious payload without AST safeguards.
**Prevention:** Always parse expressions into an AST and manually evaluate them using a whitelist of allowed mathematical operations to enforce secure and strictly deterministic computation.
