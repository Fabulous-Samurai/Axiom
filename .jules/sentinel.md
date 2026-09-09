## 2024-10-24 - Fix python eval command injection vulnerability in sandbox.py
**Vulnerability:** Command injection and arbitrary code execution possible via the use of `eval()` in Python subprocess within `scripts/sandbox.py`.
**Learning:** Python `eval()` cannot be easily sandboxed just by zeroing `__builtins__`, as arbitrary code execution and module loading is trivial (e.g. by leveraging MRO).
**Prevention:** Rely on `ast.NodeVisitor` or similar whitelisted AST evaluators for safe expression evaluation.
