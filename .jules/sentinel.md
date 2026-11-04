## 2024-06-25 - Python eval Sandbox Escape
**Vulnerability:** Untrusted expressions passed to `run_isolated_expression` in `scripts/sandbox.py` were evaluated using `eval()`.
**Learning:** `eval()` should never be used to evaluate mathematical expressions from untrusted strings, even when partially sandboxed, as it allows arbitrary code execution via standard library imports (e.g. `__import__('os').listdir('.')`).
**Prevention:** Implement a custom `ast.NodeVisitor` that explicitly checks AST nodes to evaluate operations securely rather than using `eval()` or `ast.literal_eval()`.
