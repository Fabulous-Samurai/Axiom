## 2024-05-18 - Fix command injection and exception swallowing
**Vulnerability:** Subprocess calls used `shell=True`, posing command injection risks, and bare `except:` blocks swallowed critical exceptions.
**Learning:** In utility scripts handling string-based commands or generic errors, neglecting explicit exception types or shell escaping defaults to unsafe execution, increasing vulnerability surface.
**Prevention:** Explicitly use `shell=False` in `subprocess.run()`, prefer list arguments over strings, employ `shlex.join()` for logging list arguments, and always catch specific exception types (like `Exception`) rather than using bare `except:`.
