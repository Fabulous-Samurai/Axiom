## 2024-05-24 - [Fix Command Injection in Sandbox]
**Vulnerability:** Found an RCE vulnerability in `scripts/sandbox.py` where raw `eval()` was used on unsanitized user inputs, permitting arbitrary python code execution (like `__import__("os")`).
**Learning:** The usage was primarily done due to ease of evaluating string-based arithmetic and functions, but without AST restrictions, this became dangerous in the sandbox constraints.
**Prevention:** Always restrict dynamically evaluated strings using AST parsers or safe alternatives such as `ast.literal_eval` or custom `ast.NodeVisitor` based evaluations when complex arithmetic handling is needed instead of `eval`.
