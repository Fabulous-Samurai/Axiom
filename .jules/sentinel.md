## $(date +%Y-%m-%d) - Command Injection via eval()
**Vulnerability:** The sandbox scripts rely on Python's `eval()` function to execute expressions, leading to a command injection vulnerability where a payload like `__import__('os').listdir('.')` executes successfully.
**Learning:** `eval()` does not safely isolate user input without heavy restrictions, and built-in functions allow escaping limited environments. A robust AST-based evaluator avoids executing arbitrary code.
**Prevention:** Use an AST parsing strategy with restricted operations instead of `eval()` to guarantee secure evaluation.
