## 2024-06-04 - [Replace eval with SafeMathEvaluator]
**Vulnerability:** Use of `eval()` in `scripts/sandbox.py` allows arbitrary code execution (code injection) via user-controlled input in subprocess execution.
**Learning:** Using `eval()` even in restricted subprocesses is unsafe. Code execution needs to be strictly constrained to safe operations.
**Prevention:** Use an AST-based evaluator with a whitelist of safe nodes (e.g., `SafeMathEvaluator` using `ast.parse`) instead of `eval()` to prevent execution of malicious code.
