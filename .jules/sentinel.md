## 2026-08-02 - Securing eval() and Subprocess
**Vulnerability:** Arbitrary code execution vulnerability via unfiltered eval().
**Learning:** To mitigate code injection risks in Python tools without breaking general sandbox functionality, restrict the `eval()` environment using a safe whitelist for `__builtins__`. Completely stripping `__builtins__` breaks standard math evaluation. Furthermore, explicitly pass `shell=False` to `subprocess.Popen` to satisfy static analyzers like SonarCloud when using list-based arguments.
**Prevention:** Restrict the `eval()` execution context using explicitly defined `__builtins__` and always use `shell=False` explicitly.
