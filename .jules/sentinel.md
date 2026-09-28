## YYYY-MM-DD - Unsafe Eval Vulnerability
**Vulnerability:** Arbitrary code execution via unsafe `eval()` on user input in sandbox.
**Learning:** Using `eval()` even for restricted use cases allows code execution via MRO traversal; a strict AST-based evaluator is required.
**Prevention:** Implement a strict AST `NodeVisitor` that explicitly whitelists safe mathematical operators and names.
