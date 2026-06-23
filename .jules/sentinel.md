## 2024-06-23 - Arbitrary Code Execution in Sandbox eval()
**Vulnerability:** The `eval` function in `scripts/sandbox.py` was used to evaluate untrusted user input without restricting `__builtins__`, allowing full execution of code like `__import__('os')`.
**Learning:** Even when run in an isolated subprocess, unrestricted `eval()` is inherently dangerous because it allows sandbox escape within the subprocess and potential environment manipulation.
**Prevention:** Always restrict the `eval()` environment by passing `{'__builtins__': {}}` for globals and `{}` for locals to mitigate dangerous execution capabilities while keeping simple evaluations functional.
