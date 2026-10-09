## YYYY-MM-DD - Fix RCE vulnerability in AXIOM sandbox
**Vulnerability:** The sandbox evaluator used Python's native `eval()` function directly to evaluate expressions, which is inherently insecure and allows arbitrary code execution via functions like `__import__('os').system()`.
**Learning:** Even in restricted environments or when expected input is math expressions, using `eval()` directly introduces severe command injection / RCE vulnerabilities. Custom AST parsers must be explicitly constructed with strict whitelists.
**Prevention:** Replace direct `eval()` calls with safe AST parsing evaluation using an `ast.NodeVisitor` that strictly whitelists permitted constants, operators, and functions (e.g., math functions), completely restricting generic system calls.
