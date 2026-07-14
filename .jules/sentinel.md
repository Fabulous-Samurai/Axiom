## 2026-07-14 - Prevent Command Injection in Developer Tools
**Vulnerability:** The `scripts/sonar_helper.py` script contained a command injection vulnerability where a user-controlled parameter (`--ide`) was passed directly to `subprocess.run(cmd, shell=True)`.
**Learning:** Even internal helper scripts pose security risks if they process unsanitized user inputs or arguments and execute them using `shell=True`.
**Prevention:** Always use list-based arguments with `shell=False` for `subprocess.run()` and `subprocess.Popen()`. When executing external commands in a cross-platform context, explicitly resolve the executable path using `shutil.which()` to ensure Python can locate and execute it securely without relying on the shell.
