
## 2026-09-26 - Sandbox Remote Code Execution (RCE) via `eval`
**Vulnerability:** The `scripts/sandbox.py` component evaluated mathematical expressions using Python's `eval()` function, which permits arbitrary code execution (RCE).
**Learning:** Using `eval` even inside what's perceived as a "sandbox" (without strict AST limits) is dangerous, as Python's dynamic nature allows escaping to `os` or other modules via builtins or MRO traversal.
**Prevention:** Instead of using `eval()`, use a custom AST evaluator (`ast.NodeVisitor`) that implements a strict whitelist for mathematical operations, functions, and nodes. Always parse mathematical input rather than executing it.
