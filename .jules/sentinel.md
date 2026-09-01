## 2024-05-24 - Command Injection in sandbox.py
**Vulnerability:** The `run_isolated_expression` function in `scripts/sandbox.py` passed unsanitized user input directly into Python's `eval()` function via a subprocess.
**Learning:** Relying on process-level isolation (like AppContainer or seccomp) is insufficient defense-in-depth if the entry point language interpreter is allowed to execute arbitrary code.
**Prevention:** Replace all uses of `eval()` with a custom, restricted Abstract Syntax Tree (AST) evaluator that only permits safe operations (e.g., mathematics and basic logic).
