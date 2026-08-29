
## 2024-05-24 - Command Injection in sandbox.py Evaluator
**Vulnerability:** Arbitrary code execution risk in sandbox environment due to the use of Python `eval()` in `scripts/sandbox.py`.
**Learning:** Building Python execution strings with `eval()` and passing them directly to subprocesses is extremely risky and can lead to immediate local file/system access.
**Prevention:** Replace `eval()` entirely with a strict AST `NodeVisitor` that only permits allowed binary/unary mathematical operations.
