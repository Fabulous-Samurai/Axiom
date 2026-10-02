## 2024-10-02 - Sandbox Remote Code Execution (RCE) via eval()
**Vulnerability:** The sandbox evaluator in `scripts/sandbox.py` used Python's `eval()` on unsanitized input to evaluate mathematical expressions, allowing arbitrary code execution (e.g., `__import__('os').listdir('.')`).
**Learning:** Even in tools supposedly designed for restricted evaluation, using raw `eval()` with just a subprocess is a critical vulnerability. Disabling builtins is also not sufficient due to MRO traversal. A strictly whitelisted AST-based parser must be used instead.
**Prevention:** Never use `eval()` for untrusted input. Use `ast.parse` and an `ast.NodeVisitor` that strictly whitelists only the allowed nodes, functions, and mathematical operators.
