## 2024-05-18 - Prevent Command Injection via eval()
**Vulnerability:** Found arbitrary code execution vulnerability via unsafe `eval()` usage in `scripts/sandbox.py`'s `run_isolated_expression`.
**Learning:** `eval()` was used as a shortcut for mathematical evaluation within a subprocess, leaving it completely vulnerable to malicious payloads if the outer sandbox failed.
**Prevention:** Replaced `eval()` with a custom AST node-whitelisting evaluator that strictly only allows mathematical operations and basic numerical constants.
