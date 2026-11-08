## 2024-11-08 - RCE via python eval in Sandbox
**Vulnerability:** Found `eval()` used directly in `scripts/sandbox.py` to evaluate user input, which can lead to Remote Code Execution (RCE).
**Learning:** Using `eval()` even in restricted subprocesses is unsafe, as it can evaluate any valid Python expression, such as `__import__('os').listdir('.')`, allowing escape from basic subprocess containment.
**Prevention:** Implement and use a strict AST evaluator via `ast.NodeVisitor` that only supports a whitelist of safe operators and mathematical functions.
