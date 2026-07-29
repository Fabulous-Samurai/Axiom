## 2026-07-29 - Command Injection Vulnerability (S2076)
**Vulnerability:** Use of shell=True in subprocess.run() in utility scripts (zenith_audit_scribe.py, sonar_helper.py).
**Learning:** Using shell=True with user-controllable input or string-based commands exposes the system to command injection.
**Prevention:** Always use shell=False, pass arguments as a list, use shlex.split() to parse command strings, and resolve executables using shutil.which().
