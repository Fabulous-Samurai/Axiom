## 2024-10-23 - [Sandbox AST Replace]
**Vulnerability:** Python `eval()` in `scripts/sandbox.py` allowed arbitrary code execution, including filesystem access via `__import__('os')`.
**Learning:** `eval()` should never be used for evaluating untrusted input, even in a sandbox, as it trivially allows escaping basic restraints.
**Prevention:** Always parse untrusted input using `ast.parse` and implement a strict `ast.NodeVisitor` that only allows explicitly approved operations and modules, rejecting anything else. Ensure defensive limits (e.g., maximum exponent values) are added to prevent DoS.
