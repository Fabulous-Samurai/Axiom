## 2026-07-06 - Unrestricted eval() in Sandbox
**Vulnerability:** The sandbox evaluation in scripts/sandbox.py used unrestricted eval() to run arbitrary expressions, allowing potential arbitrary code execution, such as accessing the file system.
**Learning:** Even within isolated subprocesses or sandboxes, passing untrusted input directly to eval() without restricting __builtins__ is dangerous and can lead to immediate sandbox escapes.
**Prevention:** Always restrict the globals and locals environment of eval() (e.g., eval(expr, {'__builtins__': {}}, {})) when evaluating untrusted input, even as a secondary defense layer.
