
## 2026-10-10 - [CRITICAL] Prevent Arbitrary Code Execution in Sandbox
**Vulnerability:** The Python Sandbox (`scripts/sandbox.py`) used `eval()` directly in a subprocess to evaluate mathematical expressions. This allowed arbitrary code execution, such as `__import__('os').listdir('.')` or `__import__('time').sleep(10)`.
**Learning:** Even when running in an isolated subprocess with timeout and memory guards, direct use of `eval()` exposes the system to potentially dangerous Python code (especially file system access and system commands).
**Prevention:** Replaced `eval()` with a custom AST evaluator (`ast.NodeVisitor` style implementation) that strictly whitelists mathematical operations and constants from the `math` module. This completely eliminates the ability to execute system commands or access arbitrary modules.
