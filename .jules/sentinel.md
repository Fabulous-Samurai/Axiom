## 2024-10-24 - Python eval() Code Injection in Sandbox
**Vulnerability:** The `run_isolated_expression` function in `scripts/sandbox.py` used `eval()` to execute untrusted user input within a subprocess, allowing arbitrary code execution (Command/Code Injection).
**Learning:** `eval()` was used as a shortcut to evaluate math expressions, but `ast.literal_eval()` does not support basic binary arithmetic (e.g., `2 + 2`), leading to the dangerous fallback.
**Prevention:** Use a custom `ast.NodeVisitor` that explicitly whitelists safe arithmetic operations (`ast.Add`, `ast.Sub`, etc.) and strictly rejects function calls (`ast.Call`) and unsupported nodes to securely evaluate math expressions.
