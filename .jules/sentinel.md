## 2024-05-23 - Command Injection in IDE Launcher
**Vulnerability:** Command injection vulnerability in `scripts/sonar_helper.py` through `subprocess.run(shell=True)` when launching the IDE.
**Learning:** Fixing `shell=True` on cross-platform scripts often breaks Windows integration because Python's `subprocess` without a shell doesn't search for `.cmd` or `.bat` executable aliases directly.
**Prevention:** Use `shutil.which()` to resolve the executable path before executing `subprocess.run(shell=False)` to safely handle Windows-specific executable wrappers while maintaining strict security constraints.
