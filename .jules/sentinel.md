## 2026-07-12 - Prevent Arbitrary Code Execution in Sandbox
**Vulnerability:** The sandbox used unfiltered eval() on untrusted input which allows arbitrary code execution via functions like __import__('os'). Subprocess isolation is an insufficient defense.
**Learning:** Python eval() can easily be exploited if globals and locals are not severely restricted, even when running inside a subprocess.
**Prevention:** Always restrict the eval() environment, explicitly removing __builtins__ ({'__builtins__': {}}) to prevent arbitrary function calls while retaining general expression parsing.
