## 2024-05-24 - [CRITICAL] Fix code injection in sandbox evaluation
**Vulnerability:** `eval()` was used on unsanitized user input in `scripts/sandbox.py`.
**Learning:** Evaluating untrusted strings using `eval()` allows arbitrary code execution.
**Prevention:** Use an AST-based `SafeMathEvaluator` with a strict whitelist of allowed nodes and route debug logs to `sys.stderr`.
