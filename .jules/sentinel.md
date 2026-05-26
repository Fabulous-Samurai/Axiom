## 2024-03-01 - [Command Injection via shell=True]
**Vulnerability:** Command injection vulnerability in `scripts/sonar_helper.py` due to using `shell=True` with user-provided `ide_cmd` in `subprocess.run()`.
**Learning:** `subprocess.run(cmd, shell=True)` with user-supplied commands allows arbitrary shell commands to be executed if the input is not sanitized.
**Prevention:** Always use `shell=False` for user-supplied inputs and resolve the executable natively with `shutil.which` to ensure commands execute correctly across platforms without invoking the shell.
