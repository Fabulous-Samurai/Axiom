## 2024-06-19 - Unfiltered eval() Code Injection in Sandbox
**Vulnerability:** The Python sandbox environment used unfiltered `eval()` allowing arbitrary code execution.
**Learning:** Even within an isolated subprocess, passing unconstrained user input to `eval()` is vulnerable because it can access standard library built-ins to break out of the sandbox.
**Prevention:** Always restrict `eval()` by explicitly passing `{'__builtins__': {}}` as globals and `{}` as locals, or ideally use an AST-based evaluator if applicable.
