## 2026-08-12 - [CRITICAL] Fix command injection in sandbox
**Vulnerability:** scripts/sandbox.py used eval() with no restrictions on __builtins__, allowing arbitrary code execution via shell commands.
**Learning:** Because it must evaluate general Python expressions, replacing eval() with a strict AST-based parser breaks sandbox functionality. We must restrict the eval() environment with a safe __builtins__ whitelist instead.
**Prevention:** Do not use unrestricted eval(). Use a restricted whitelist for __builtins__ if eval() is strictly necessary for general expressions.
