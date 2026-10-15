## 2024-10-15 - RCE in Sandbox Module
**Vulnerability:** The `scripts/sandbox.py` file evaluated user inputs with the insecure Python `eval()` built-in, enabling Remote Code Execution (RCE) via malicious strings like `__import__('os').system('...')`.
**Learning:** Even though it's called `sandbox.py` and the docstring claims restrictions, evaluating untrusted strings using `eval()` is inherently insecure unless executed in a strictly sandboxed container process (like AppContainer or Seccomp) which shouldn't be relied upon as the sole defense mechanism.
**Prevention:** Always parse untrusted input using `ast.parse` and carefully evaluate using a restrictive AST whitelist visitor (`ast.NodeVisitor`) that only supports necessary subset of features.
