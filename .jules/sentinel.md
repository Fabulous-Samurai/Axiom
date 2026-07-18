## 2026-07-18 - Prevent Sandbox Escape via Python eval()
**Vulnerability:** The sandbox subprocess allowed arbitrary code execution due to unfiltered eval() being executed via sys.executable, allowing payloads like __import__('os').listdir('.') to escape.
**Learning:** Process isolation combined with unrestricted eval() fails to provide sufficient security if the Python runtime's default built-ins aren't stripped or whitelisted.
**Prevention:** Always restrict __builtins__ using a safe whitelist when evaluating strings using Python's eval().
