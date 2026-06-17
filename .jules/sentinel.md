## 2024-06-17 - [Sandbox Command Injection via eval]
**Vulnerability:** In `scripts/sandbox.py`, `eval()` was used on untrusted input directly inside the subprocess command string, allowing unrestricted code execution.
**Learning:** Even within an isolated subprocess, passing unfiltered user input to `eval()` is a vulnerability because the subprocess itself becomes compromised.
**Prevention:** When using `eval()`, explicitly restrict the environment globals and locals, especially stripping out `__builtins__`, to sandbox the python execution context.
