## 2024-08-30 - Fix Command Injection in sandbox.py
**Vulnerability:** AXIOM expression evaluator used unsafe `eval()` directly in a subprocess, leading to command injection (e.g. `__import__('os').system(...)`).
**Learning:** Restricting `__builtins__` is ineffective. Evaluating string expressions securely requires a full AST-based parsing approach to strictly whitelist permitted operations.
**Prevention:** Use a strict AST `NodeVisitor` (e.g. `safe_eval.py`) to parse expressions into an AST and explicitly handle permitted nodes (arithmetic, comparators, constants) instead of relying on runtime sandboxing like `eval()`.
