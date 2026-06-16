## 2024-10-24 - [CRITICAL: Arbitrary Code Execution in Sandbox]
**Vulnerability:** The sandbox evaluation mechanism (`scripts/sandbox.py`) used Python's `eval()` on unvalidated user input, allowing arbitrary code execution even with process isolation.
**Learning:** Process isolation (like seccomp or AppContainer) is an important defense-in-depth layer, but using `eval()` on untrusted input is fundamentally unsafe. Isolation alone does not stop execution of arbitrary Python code within the sandbox itself.
**Prevention:** Always use safe, AST-based evaluators (like `SafeMathEvaluator` restricted to `ast.Constant`) or secure parsers for mathematical expressions instead of `eval()`.
