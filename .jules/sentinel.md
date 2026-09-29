## YYYY-MM-DD - Custom AST Evaluation for Math Expressions
**Vulnerability:** Arbitrary code execution in `scripts/sandbox.py` via `eval()` that evaluated user-provided math expressions.
**Learning:** In sandbox environments, relying on `eval()` with disabled built-ins (`{'__builtins__': {}}`) is insufficient. Attackers can traverse the Method Resolution Order (MRO) (e.g., `().__class__.__bases__[0].__subclasses__()`) to reacquire dangerous modules like `os`.
**Prevention:** Implement a strict, custom AST-based evaluator (using `ast.parse` and `ast.NodeVisitor`) that exclusively whitelists permitted mathematical functions and operations, avoiding Python's native `eval()` entirely.
