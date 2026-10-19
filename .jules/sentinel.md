## 2024-05-24 - Sandbox RCE and DoS vulnerability via eval()

**Vulnerability:** The `run_isolated_expression` in `scripts/sandbox.py` was evaluating arbitrary expressions using `eval()` in a subprocess. This allowed command injection, arbitrary file access, and DoS (e.g. Memory Exhaustion via large list multiplications).

**Learning:** Using `eval` without strict ast-based evaluation leaves the sandbox completely exposed even if executed in a subprocess. Passing user input to a subprocess that runs `eval` enables arbitrary code execution on the system.

**Prevention:** Always use safe abstract syntax tree (AST) evaluators `ast.NodeVisitor` explicitly for arithmetic evaluations instead of `eval()`. Explicitly handle all allowed nodes, implement operator limits (like preventing large power operators or list allocations) and ensure operand types are verified.
