## 2025-02-23 - Command Injection via subprocess.run
**Vulnerability:** Found multiple instances where `subprocess.run` was called with `shell=True` and unvalidated strings.
**Learning:** Using `shell=True` exposes the application to command injection if paths or command arguments are ever user-controlled. Specifically, on Windows environments, directly switching to `shell=False` without resolving batch file aliases like `code` (e.g. VS Code's `code.cmd`) breaks functionality because `CreateProcess` only auto-resolves `.exe`.
**Prevention:** Always use `shell=False` with command arrays. When relying on CLI tools that might be `.cmd` or `.bat` files on Windows, strictly resolve the path first using `shutil.which(cmd)`.
