## 2024-06-02 - Command Injection Mitigations
**Vulnerability:** Using shell=True in Python subprocess calls exposes the application to command injection, and passing lists with shell=True behaves inconsistently across operating systems.
**Learning:** When refactoring subprocess calls to shell=False, Windows requires shutil.which() to correctly resolve executables like 'code' without relying on the shell to find batch wrappers. Additionally, shlex.split() incorrectly handles Windows file paths.
**Prevention:** Always use shell=False with natively defined command lists, use shutil.which() to resolve cross-platform executables, and use sys.executable for invoking Python.
