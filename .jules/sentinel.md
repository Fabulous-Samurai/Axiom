## 2026-08-03 - Command Injection in Python Sandbox
**Vulnerability:** The sandbox evaluation script used python's eval() without restricting the execution environment, allowing arbitrary code execution via built-ins like __import__.
**Learning:** Using eval() on user input is highly dangerous. Process isolation (via sys.executable -c) combined with eval() is not enough if the eval environment is unrestricted.
**Prevention:** Always restrict the globals/locals environment of eval() using a safe __builtins__ whitelist (e.g., {'__builtins__': {'abs': abs, ...}}).
