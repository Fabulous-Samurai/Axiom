## 2024-08-28 - Replace `eval()` with Custom AST Evaluator
**Vulnerability:** The Python `eval()` function is being used to evaluate user input dynamically in `scripts/sandbox.py`. This poses a command injection and remote code execution (RCE) risk. Even restricted `eval` calls can often be bypassed.
**Learning:** Limiting `__builtins__` is not an effective anti-pattern for safe code evaluation since Python's underlying architecture still allows access to various builtins. To safely evaluate mathematical or logic expressions without external dependencies, one should construct a custom AST evaluator.
**Prevention:** Always use safe parsed structures (like the `ast` module with a strict whitelisting `NodeVisitor`) for expression evaluations instead of raw `eval()`.
