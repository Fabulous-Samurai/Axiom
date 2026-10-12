## YYYY-MM-DD - Remote Code Execution (RCE) via `eval()` in Sandbox
**Vulnerability:** The `scripts/sandbox.py` script used `eval()` directly on user-provided input `code = f"import os; print(eval({repr(expression)}))"` without sufficient validation, leading to a sandbox escape.
**Learning:** Python's built-in `eval()` function evaluates arbitrary Python expressions. When used directly on unsanitized user input, it allows full execution of code like `__import__('os').system('...')`.
**Prevention:** Never use `eval()` on unsanitized or untrusted input. Instead, use a safe abstract syntax tree (AST) evaluator (like `ast.literal_eval` or a custom `ast.NodeVisitor`) with a strict whitelist of allowed operators and functions to parse and evaluate mathematical expressions safely.
