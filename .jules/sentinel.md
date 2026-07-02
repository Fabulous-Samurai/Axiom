## 2024-05-18 - Unrestricted eval() in Sandbox Evaluator
**Vulnerability:** Arbitrary code execution via unrestricted `eval()` in `scripts/sandbox.py`, allowing sandbox escape and host system access (e.g. via `__import__`).
**Learning:** Using process isolation (subprocess) is not sufficient when the spawned process evaluates untrusted input with a fully privileged Python environment.
**Prevention:** Always restrict the execution environment for `eval()` using `{'__builtins__': {}}` and empty locals `{}` when evaluating untrusted expressions, even inside isolated subprocesses.
