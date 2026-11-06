## 2025-02-28 - Insecure Sandbox Evaluator
**Vulnerability:** Arbitrary code execution via `eval()` in `scripts/sandbox.py` allowing sandbox escapes (e.g. `__import__('os').listdir('.')`).
**Learning:** Even within a resource-monitored subprocess, using raw `eval()` to execute user expressions is inherently insecure in Python because it evaluates any valid python expression, bypassing outer sandbox limits if the expression imports system libraries.
**Prevention:** Always use safe parsing like `ast.literal_eval` or implement a restrictive `ast.NodeVisitor` that explicitly whitelists safe operations and data types when evaluating user-provided strings.
