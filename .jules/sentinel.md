## 2024-05-18 - [Fix arbitrary code execution in sandbox via eval()]
**Vulnerability:** `scripts/sandbox.py` was using Python's `eval()` within a subprocess to run user-provided expressions, allowing a trivially easy sandbox escape and arbitrary code execution.
**Learning:** Never pass untrusted strings to `eval()`, even inside a separate subprocess, unless using OS-level constructs like seccomp or AppContainer. Python's `eval()` executes the code natively in the Python interpreter.
**Prevention:** Use an AST-based parser (`ast.parse`) with strict node validation (blocking `ast.Call` and `ast.Attribute`) to ensure only mathematically safe expressions are evaluated.
