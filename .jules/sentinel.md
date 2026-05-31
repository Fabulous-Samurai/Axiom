## 2024-05-31 - Command Injection via Unsanitized IDE Argument
**Vulnerability:** `scripts/sonar_helper.py` passes the user-controlled `--ide` argument directly to `subprocess.run` with `shell=True`, allowing arbitrary command execution.
**Learning:** Using `shell=True` with unvalidated user input creates severe command injection vulnerabilities. `shell=True` is often used as a crutch for finding executables like VS Code's `code.cmd` on Windows.
**Prevention:** Use `shell=False` with commands defined as lists. To resolve executables cross-platform without a shell, use `shutil.which()`.
