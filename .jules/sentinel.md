## 2024-06-21 - Command Injection via shell=True
**Vulnerability:** Found `subprocess.run` with `shell=True` using a list as `cmd` in `scripts/sonar_helper.py`, which is dangerous if `ide_cmd` or `file_path` contains malicious content.
**Learning:** Passing a list to `subprocess.run` with `shell=True` doesn't protect against command injection if the first element or arguments are attacker-controlled.
**Prevention:** Use `shell=False` to prevent the shell from parsing command arguments. When wrapping system utilities like `code` (VS Code), explicitly resolve the executable path using `shutil.which` to ensure cross-platform compatibility when `shell=False` is set.
