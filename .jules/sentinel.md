## YYYY-MM-DD - [Title]
**Vulnerability:** [What you found]
**Learning:** [Why it existed]
**Prevention:** [How to avoid next time]
## 2024-11-11 - Replaced eval() with AST NodeVisitor for secure math evaluation
**Vulnerability:** The sandbox used python's native `eval()` which enables RCE if an attacker inputs malicious python code such as `__import__('os').system(...)`.
**Learning:** `eval()` is intrinsically unsafe for untrusted strings. While AppContainer/seccomp provide defense in depth, application-level sandboxing should avoid inherently dangerous primitives.
**Prevention:** Always parse expressions using `ast.parse` and traverse the AST node safely via a whitelist using `ast.NodeVisitor`, restricting the allowed operators and types.
