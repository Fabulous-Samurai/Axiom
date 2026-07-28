## 2026-07-28 - Command Injection in IDE execution
**Vulnerability:** Command injection due to subprocess.run using shell=True with user-provided arguments in sonar_helper.py.
**Learning:** Even internal helper scripts can be vulnerable if they pass unvalidated arguments to a shell.
**Prevention:** Always use shell=False with subprocess.run and safely parse arguments using shlex.split when handling user inputs.
