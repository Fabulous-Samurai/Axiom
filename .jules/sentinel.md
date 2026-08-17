## 2026-08-17 - Prevent eval() command injection via custom AST parsing
**Vulnerability:** AXIOM expression evaluator used unrestricted `eval()` in a subprocess, allowing arbitrary Python execution and command injection.
**Learning:** Restricting the `eval()` environment with a `__builtins__` whitelist is an ineffective sandboxing technique that can be bypassed via object introspection (e.g., `().__class__.__bases__[0].__subclasses__()`). A truly secure approach requires parsing the AST and strictly evaluating safe mathematical nodes.
**Prevention:** Never use `eval()` for untrusted input. Use an AST-based parser to explicitly walk and evaluate allowed nodes.
