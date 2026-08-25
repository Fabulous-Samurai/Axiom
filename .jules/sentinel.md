## 2024-05-23 - Arbitrary Code Execution via eval()
**Vulnerability:** A Remote Code Execution (RCE) / Command Injection vulnerability was present in `scripts/sandbox.py` because it evaluated user-provided expressions dynamically using `eval()` within a child Python process.
**Learning:** `eval()` allows execution of arbitrary Python code, such as executing shell commands via `__import__('os').system()`. Restricting Python evaluation safely is practically impossible by merely cleaning the string.
**Prevention:** Always use `ast.parse` in combination with an AST NodeVisitor designed to whitelist specific node types and operations (like arithmetic operators), avoiding executing logic implicitly or unsafely.
