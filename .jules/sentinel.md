## 2026-07-19 - Prevent Arbitrary Code Execution in Python eval()
**Vulnerability:** Unrestricted eval() in scripts/sandbox.py allowed arbitrary code execution, such as using __import__ to access the OS and escape the sandbox.
**Learning:** Even when run in an isolated subprocess, an unrestricted eval() without restricted globals/locals (__builtins__) presents a critical Remote Code Execution (RCE) risk. Escaping quotes via `%r` format strings instead of f-strings avoids curly brace syntax errors.
**Prevention:** Always restrict the execution context in eval() by providing safe whitelists for __builtins__ (e.g., `{'__builtins__': {'abs': abs, ...}}`) and use process isolation/timeouts as defense in depth.
