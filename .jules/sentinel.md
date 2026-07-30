## 2026-07-30 - Command Injection in Subprocess
**Vulnerability:** The `scripts/sonar_helper.py` tool used `subprocess.run(cmd, shell=True)` with a user-provided IDE command, allowing arbitrary shell command execution.
**Learning:** Even when passing a list to `subprocess.run`, if `shell=True` is set, the first element can contain shell metacharacters and be executed by the shell, leading to command injection.
**Prevention:** Always use `shell=False` with list-based arguments. For user-provided command strings, safely parse them using `shlex.split()` and explicitly resolve the executable using `shutil.which()` for cross-platform compatibility without the shell.
