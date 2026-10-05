## YYYY-MM-DD - [Arbitrary Code Execution in Sandbox]
**Vulnerability:** The sandbox evaluator `run_isolated_expression` in `scripts/sandbox.py` used Python's `eval()` on unsanitized input to evaluate mathematical expressions, allowing arbitrary code execution (e.g. `__import__('os').listdir('.')`).
**Learning:** Even when disabling `__builtins__` or in a restricted subprocess, Python's `eval` can be bypassed via MRO traversal or malicious imports. Relying on `eval` for evaluating arbitrary expressions is inherently dangerous.
**Prevention:** Use a strict AST-based whitelisting approach (e.g., `ast.NodeVisitor`) to safely evaluate mathematical expressions, restricting evaluation only to safe arithmetic operations and whitelisted mathematical functions and constants.
