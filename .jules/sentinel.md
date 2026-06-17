## 2024-06-17 - [Sandbox Command Injection]
**Vulnerability:** In `scripts/sandbox.py`, `eval()` was used on untrusted input directly, even inside an isolated subprocess. Subprocess isolation shouldn't be the only layer of defense.
**Learning:** Subprocess isolation (e.g., AppContainer, seccomp) should only act as a secondary defense layer for sandboxes; it is fundamentally unsafe to use Python's `eval()` on untrusted input even within an isolated subprocess.
**Prevention:** Implement an AST-based `SafeMathEvaluator` that whitelists safe nodes (arithmetic, constants) to mitigate code injection risks instead of `eval()`.
