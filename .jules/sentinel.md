## 2026-07-13 - Secure Restricted eval within Subprocess
**Vulnerability:** The sandbox subprocess used eval() with a completely open environment (e.g., globals/locals allowed __builtins__).
**Learning:** Process isolation (AppContainer/seccomp) is only a secondary defense layer; it is fundamentally unsafe to use Python's eval() on untrusted input even within an isolated subprocess, as an attacker could execute arbitrary commands via __import__('os'). However, completely stripping __builtins__ breaks standard math.
**Prevention:** Always restrict __builtins__ from eval() contexts using a safe whitelist (e.g., {'__builtins__': {'abs': abs, 'min': min, 'max': max, 'int': int, 'float': float}}) to prevent code execution while retaining pure math/expression parsing. Additionally, ensure subprocess.Popen explicitly uses shell=False to prevent command injection (S2076).

## 2026-07-13 - Securing CI Supply Chain (Pre-Commit)
**Vulnerability:** The CI pipeline relied on an unmaintained and failing third-party GitHub hook (pwaller/pre-commit-clang-format).
**Learning:** Using unverified or stale third-party repositories for code execution (even for formatting) introduces supply-chain risks and fragility into the CI pipeline.
**Prevention:** Replace unmaintained third-party tools with official, verifiable mirrors (e.g., pre-commit/mirrors-clang-format) pinned to specific stable versions to secure the supply chain and unblock CI.
