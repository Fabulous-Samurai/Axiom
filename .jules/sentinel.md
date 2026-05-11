
## 2024-05-11 - [HIGH] Fix command injection vulnerability
**Vulnerability:** Command injection risk identified in `scripts/sonar_helper.py` and `scripts/zenith_audit_scribe.py` due to the use of `subprocess.run` with `shell=True`. User-provided inputs passed to shell commands could lead to arbitrary command execution if they are not correctly sanitized.
**Learning:** `shell=True` was used for executing subprocesses, which creates unnecessary security risks even in internal scripts.
**Prevention:** Instead of passing string commands with `shell=True`, developers should pass command arguments as lists and use `shell=False`. Use `shlex.split` for safely parsing command strings, and `sys.executable` when invoking the current Python interpreter.
