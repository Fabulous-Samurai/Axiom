## 2026-07-07 - Securing Subprocess eval()

**Vulnerability:** Arbitrary code execution via unrestricted `eval()` passed to an isolated subprocess in `scripts/sandbox.py`. Although the subprocess offers isolation, `eval()` execution allows easy file system access or breaking sandbox boundaries before isolation constraints are fully effective.

**Learning:** When using Python's `eval()` inside a subprocess, the string injected via `sys.executable -c` must be executed with a strictly restricted environment (e.g., `{'__builtins__': {}}`). Furthermore, using f-strings for string interpolation when curly braces `{}` are involved is unsafe (causes `KeyError`); use `%r` instead. Route any exceptions to `sys.stderr` to prevent sandbox logic from misinterpreting `sys.stdout` logs as valid output.

**Prevention:** Always restrict the `eval()` environment even if process isolation is in place, and avoid f-strings when building executable code containing curly braces.
