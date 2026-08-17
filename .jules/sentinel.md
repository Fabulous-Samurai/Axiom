## 2026-08-17 - Prevent eval() command injection in Python sandbox
**Vulnerability:** AXIOM expression evaluator used unrestricted `eval()` in a subprocess, allowing arbitrary Python execution.
**Learning:** To support general mathematical expressions while avoiding command injection, `eval()` must have a strict whitelist of allowed `__builtins__`. Completely replacing it with AST parsing breaks expected sandbox functionality.
**Prevention:** Always pass a restricted `__builtins__` dictionary to `eval()` when executing untrusted expressions.
