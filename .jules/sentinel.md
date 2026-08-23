## 2024-05-18 - Isolated Process Escape via Insecure eval()
**Vulnerability:** The sandbox evaluator in `scripts/sandbox.py` relied on `eval()` to execute dynamically generated code within an isolated subprocess, which could be bypassed with `__import__('os').system('...')` allowing for Arbitrary Code Execution (ACE) before OS-level sandbox (seccomp/AppContainer) protections.
**Learning:** Using `eval()` combined with `repr()` formatting provides false security. True security requires restricting the execution engine at the Abstract Syntax Tree (AST) level, completely preventing unauthorized nodes (like function calls).
**Prevention:** Implement an `ast.NodeVisitor` that strictly whitelists operators and literal types. Never use `eval()` on unsanitized or user-provided input, even when wrapped in a subprocess.

## 2024-05-18 - Supply Chain: Actions Node.js 20 Deprecation Warnings Break SonarQube
**Vulnerability:** Deprecated GitHub Actions referencing Node.js 20 trigger warnings on the pipeline, which subsequently causes strict code analysis tools (like SonarCloud) to fail their Quality Gate purely due to the emitted pipeline warnings.
**Learning:** Security scanners evaluate the health of the entire CI execution log, not just the application code. Neglecting to update CI dependencies causes cascading failures that block merges and valid security fixes.
**Prevention:** Always bump GitHub action tags/SHAs to actively maintained versions that support the current runner environment (Node 24).
