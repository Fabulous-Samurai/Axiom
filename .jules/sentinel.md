## 2024-05-24 - Replace vulnerable eval() with SafeMathEvaluator
**Vulnerability:** Arbitrary code execution via Python's built-in `eval()` function used to evaluate math expressions in tools/visualization/advanced_3d_visualization.py.
**Learning:** Even with an empty `__builtins__` dict, `eval()` can be exploited via object introspection (`__class__`, `__subclasses__`) to achieve sandbox escape.
**Prevention:** Implement an AST-based parser that whitelists safe mathematical operations and explicitly rejects `ast.Attribute` to prevent object introspection.
