## 2026-08-02 - Securing eval() and Subprocess
**Vulnerability:** Arbitrary code execution vulnerability via unfiltered eval() and potential command injection via Popen.
**Learning:** Replacing `eval()` completely with AST-based parsers can break sandboxes designed to evaluate general expressions. Instead, we must mitigate the risk by restricting `__builtins__` to a whitelist of safe functions, and always pass `shell=False` explicitly to `subprocess.Popen` to satisfy static analyzers like SonarCloud.
**Prevention:** Restrict the `eval()` execution context using explicitly defined `__builtins__` and always use `shell=False` with list-based arguments in subprocesses.
