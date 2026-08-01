## 2026-08-01 - Securing Subprocess eval() Sandboxes
**Vulnerability:** Arbitrary code execution vulnerability via unrestricted `eval()` in a subprocess used as a math sandbox.
**Learning:** Using `eval()` even inside an isolated subprocess is unsafe if not properly restricted, as builtins like `__import__` can be leveraged. Also, using f-strings to inject code strings with `{}` inside stringified Python code can lead to syntax errors.
**Prevention:** Restrict `eval()` by providing a safe whitelist for `__builtins__` in the `globals` dictionary, and use string formatting (e.g. `%r`) to inject strings safely. Explicitly set `shell=False` in `subprocess.Popen`.
