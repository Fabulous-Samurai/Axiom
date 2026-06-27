## 2024-06-27 - [Sandbox Python eval Injection]
**Vulnerability:** Arbitrary code execution in Python `eval()` function inside the sandbox evaluator.
**Learning:** `eval()` was called with `eval(expression)` allowing arbitrary imports and system commands. `run_isolated_expression` spawns a subprocess for the expression, but the isolated environment inside the subprocess lacked restrictions, exposing the host to malicious payloads like `__import__('os').listdir('.')`.
**Prevention:** Always restrict the `eval` environment by stripping out `__builtins__` when dealing with untrusted user input, even inside isolated processes. E.g., `eval(expr, {'__builtins__': {}}, {})`.
