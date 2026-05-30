## 2025-02-12 - Fix Arbitrary Code Execution in Sandbox
**Vulnerability:** The sandbox evaluation used `eval()` which allows arbitrary python code execution, such as `__import__('os').system(...)`.
**Learning:** `eval()` should never be used on untrusted input, even if run in a subprocess.
**Prevention:** Use an AST-based parser with a whitelist of safe nodes (like `SafeMathEvaluator`) to safely parse and evaluate untrusted mathematical expressions.
