## 2026-07-13 - Secure Restricted eval within Subprocess
**Vulnerability:** The sandbox subprocess used eval() with a completely open environment (e.g., globals/locals allowed __builtins__).
**Learning:** Process isolation (AppContainer/seccomp) is only a secondary defense layer; it is fundamentally unsafe to use Python's eval() on untrusted input even within an isolated subprocess, as an attacker could execute arbitrary commands via __import__('os').
**Prevention:** Always explicitly strip __builtins__ from eval() contexts (e.g., eval(expr, {'__builtins__': {}}, {})) to prevent code execution while retaining pure math/expression parsing.
