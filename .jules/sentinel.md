## 2024-06-14 - Sandbox Remote Code Execution Prevention
**Vulnerability:** The sandbox evaluator used `eval()` to parse user input, which permitted arbitrary code execution despite subprocess isolation.
**Learning:** Subprocess isolation (like AppContainer or seccomp) should only be a secondary defense layer; `eval()` is fundamentally unsafe for untrusted input.
**Prevention:** Use an AST-based evaluator (`ast.parse`) with an explicit whitelist of safe nodes (e.g., arithmetic, constants) and `ast.Constant` for numbers/strings. Ensure computationally heavy tests evaluate math expressions without exposing `sys.set_int_max_str_digits(0)`.
