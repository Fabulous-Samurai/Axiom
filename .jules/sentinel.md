## 2023-10-31 - [Sandbox Eval Vulnerability]
**Vulnerability:** Arbitrary code execution in `scripts/sandbox.py` due to the use of Python's `eval()` function to evaluate untrusted expressions.
**Learning:** `eval()` was used directly on user input inside a subprocess, allowing potential sandbox escapes and arbitrary execution despite the process monitoring.
**Prevention:** Avoid `eval()` completely for untrusted mathematical expressions. Use a custom `ast.NodeVisitor` approach to safely evaluate expressions, which explicitly allows only specific arithmetic operations and types, completely rejecting attributes, imports, and most function calls.
