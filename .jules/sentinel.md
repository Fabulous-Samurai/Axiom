## 2024-03-24 - Arbitrary Code Execution via eval()
**Vulnerability:** The sandbox script `scripts/sandbox.py` was using Python's `eval()` to execute expressions, leading to arbitrary code execution (ACE) and sandbox escape.
**Learning:** `eval()` is fundamentally insecure for untrusted inputs and cannot be sufficiently constrained.
**Prevention:** Use `ast.parse` with `mode='eval'` and a strict custom `ast.NodeVisitor` to safely evaluate only allowed subset of operations and reject everything else explicitly.
