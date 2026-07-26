## 2026-07-26 - Command Injection via subprocess.run
**Vulnerability:** Command injection in script due to passing unsanitized user arguments to `subprocess.run` with `shell=True`.
**Learning:** Using `shell=True` with `subprocess.run` makes the code vulnerable to command injection when passing unsanitized arguments.
**Prevention:** Always use `shell=False` in `subprocess.run` combined with `shlex.split()` for command strings and `shutil.which()` for executable resolution to securely execute shell commands cross-platform.
