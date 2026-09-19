## 2024-05-18 - Prevent Arbitrary Code Execution in Sandbox Evaluation
**Vulnerability:** The sandbox module `scripts/sandbox.py` uses python `eval()` which allows arbitrary code execution via `__import__('os').listdir('.')` and similar commands.
**Learning:** Even though the function was intended as an "isolated expression" test, passing user input directly into `eval()` without any restrictions leads to severe RCE (Remote Code Execution) vulnerabilities.
**Prevention:** Replace `eval()` with `ast.literal_eval` or a custom AST-based math evaluator when only mathematical or basic operations are needed.
