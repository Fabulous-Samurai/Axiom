## 2024-05-18 - [Fix arbitrary code execution in visualization eval]
**Vulnerability:** Arbitrary code execution via `eval` in `tools/visualization/advanced_3d_visualization.py` which allowed malicious inputs like `__import__('os').system(...)`.
**Learning:** Emptying `__builtins__` in `eval()` is insufficient to prevent code execution because object introspection (e.g., using `__class__`, `__subclasses__` on allowed objects) or attributes can still be accessed to escape the sandbox.
**Prevention:** Use an AST parsing approach (like `SafeMathEvaluator`) that validates every node against a strict whitelist of safe operations before passing the expression to `eval`.
