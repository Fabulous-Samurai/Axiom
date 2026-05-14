## 2024-05-14 - Fix Command Injection Vulnerability
**Vulnerability:** In `scripts/sonar_helper.py` and `scripts/zenith_audit_scribe.py`, user input or un-sanitized commands were passed to `subprocess.run(..., shell=True)`. This could allow attackers to execute arbitrary shell commands (command injection).
**Learning:** Using `shell=True` with `subprocess` functions is a major security risk, especially when it includes dynamically constructed commands or arguments (like `ide_cmd` from command line in `sonar_helper.py`).
**Prevention:** Avoid `shell=True` whenever possible. Use a list of arguments instead of a string to invoke processes, and if a specific executable needs to be resolved on Windows without `shell=True` (like `code.cmd`), use `shutil.which()`.
