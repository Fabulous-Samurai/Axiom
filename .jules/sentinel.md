## 2026-05-17 - [CRITICAL] Fix command injection in subprocess.run
**Vulnerability:** Found `subprocess.run` with `shell=True` using unsanitized string concatenation for command execution in `scripts/sonar_helper.py` and `scripts/zenith_audit_scribe.py`. This opens the application to severe command injection risks.
**Learning:** `shell=True` creates an unnecessary attack vector when arguments are user-controlled or dynamically generated. Resolving executables via `shutil.which` and providing a native Python list of command tokens is not only safer but often more portable.
**Prevention:** Always use `shell=False` combined with standard list argument formats (e.g., `["java", "-jar", "file.jar"]`) and locate required executable paths using `shutil.which()` for cross-platform robustness.
