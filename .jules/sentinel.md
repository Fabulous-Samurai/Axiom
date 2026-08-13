## 2026-08-13 - [CRITICAL] Command Injection in Sandbox eval()
**Vulnerability:** Unrestricted use of eval() inside scripts/sandbox.py allowed execution of arbitrary Python code like __import__('os').
**Learning:** Using eval() without restricting __builtins__ provides a trivial sandbox escape path.
**Prevention:** Always pass a strictly curated whitelist (e.g., {'__builtins__': safe_builtins}) as the globals dictionary when using eval() for expression evaluation.
