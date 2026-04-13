
## 2025-02-28 - [CRITICAL] Fix command injection vulnerability in 3D visualization
**Vulnerability:** Command injection / Remote Code Execution via `eval()` parsing unvalidated strings in `Advanced3DVisualization` interactive visualization functions.
**Learning:** Hardcoded dictionary whitelists in `eval(func, {"__builtins__": {}}, safe_dict)` do not effectively prevent sandbox escapes, especially when mathematical parsing requires external inputs like UI fields or file reads.
**Prevention:** Always use safe abstract syntax tree (AST) parsing (e.g., `ast.parse` evaluated safely on allowed nodes, or libraries like `numexpr`) and strictly reject operations like `ast.Attribute` or `ast.Call` (unless calling explicitly whitelisted safe functions).
