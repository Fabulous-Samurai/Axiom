## 2024-05-15 - Arbitrary Code Execution in Sandbox
**Vulnerability:** The `scripts/sandbox.py` script was executing user-provided mathematical expressions using raw Python `eval()`, allowing arbitrary code execution (e.g. `__import__('os').listdir('.')`).
**Learning:** Even in tools labelled "sandbox" or scripts ostensibly used for evaluating simple arithmetic, native `eval()` is universally dangerous without restrictions. A subprocess boundary is not enough to protect the host filesystem or system environment if the evaluator has full Python access.
**Prevention:** Never use `eval()` on unsanitized user input. Always use an AST-based parser (`ast.parse`) combined with a restrictive visitor pattern (`ast.NodeVisitor`) to enforce an explicit allowlist of safe operations and node types.
