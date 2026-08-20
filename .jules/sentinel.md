## 2024-08-20 - [Command Injection via eval() in Sandbox]
**Vulnerability:** scripts/sandbox.py used Python's `eval()` to evaluate arbitrary user-provided AXIOM expressions in a subprocess, leading to severe command injection and sandbox escape.
**Learning:** Restricting `eval()` execution via dictionary scoping or basic filtering is an ineffective anti-pattern; it must be completely replaced for evaluating untrusted input.
**Prevention:** Replace `eval()` entirely with a custom AST-based mathematical evaluator (e.g., using `ast.parse` and a strict `NodeVisitor` that only permits safe constant values and specific binary operations) when evaluating user expressions.
