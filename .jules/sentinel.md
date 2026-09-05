## 2025-02-28 - Block Arbitrary Code Execution in Sandbox
**Vulnerability:** The sandbox module used Python's built-in `eval()` function to execute expressions, allowing arbitrary code execution (like file system escapes) because `eval()` can execute complex, malicious strings.
**Learning:** Even within bounded timeouts, `eval()` is fundamentally unsafe for untrusted input. It allows trivial sandbox escapes like accessing `__import__('os')`.
**Prevention:** Replaced `eval()` with a custom AST-based parser (`ast.parse`) combined with a strict safelist evaluator (`ast.NodeVisitor` style) that explicitly rejects dangerous nodes like `ast.Attribute` and limits available functions.
