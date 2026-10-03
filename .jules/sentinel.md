## YYYY-MM-DD - [Remote Code Execution in Sandbox Evaluation]
**Vulnerability:** The sandbox evaluation `run_isolated_expression` evaluated python code directly via `eval()` inside a subprocess, allowing Remote Code Execution and unrestricted access to the file system (e.g. `__import__('os').listdir('.')`).
**Learning:** Python's built-in `eval()` cannot be properly restricted to block arbitrary code execution, and using string concatenation for python scripts executes raw system calls.
**Prevention:** Implement a secure Abstract Syntax Tree (AST) parser via `ast.parse` and restrict evaluations using a `ast.NodeVisitor` that whitelists only mathematical operations, functions, and numeric constants.
