## 2026-09-11 - [Arbitrary Code Execution in 3D Visualization Eval]
**Vulnerability:** The Advanced 3D Visualization tool used `eval()` to parse user-provided mathematical functions and expressions. Although `{"__builtins__": {}}` was used, arbitrary code could still be executed using MRO traversal like `().__class__.__bases__[0].__subclasses__()`.
**Learning:** `eval()` should never be used for parsing user expressions even with an empty builtins dictionary, because attributes traversal (MRO) allows an attacker to find classes and modules.
**Prevention:** Use `ast.parse` and whitelist exact mathematical operations and functions when parsing expressions to safely prevent code execution while maintaining functionality.
