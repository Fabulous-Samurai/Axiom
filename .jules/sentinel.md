
## 2026-08-10 - Restricting eval() in Sandboxes
**Vulnerability:** Arbitrary code execution via `eval()` in `scripts/sandbox.py`.
**Learning:** To mitigate code injection risks in Python sandboxes without breaking support for general expressions, restrict the `eval()` environment using a safe whitelist for `__builtins__`. Replacing `eval()` completely with an AST-based parser will break test suites that rely on evaluating general expressions.
**Prevention:** Use restricted `__builtins__` for `eval()` instead of unconstrained `eval()` or `ast.literal_eval()` when mathematical operations or standard functions are necessary.
