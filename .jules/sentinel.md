## 2024-03-22 - [CRITICAL] Prevent Command Injection via eval() in 3D Visualization
**Vulnerability:** Unsafe usage of eval() in tools/visualization/advanced_3d_visualization.py allowed arbitrary Python code execution (e.g., __import__('os')) despite passing an empty __builtins__ dictionary.
**Learning:** Overriding __builtins__ to empty does not fully secure eval(), as object introspection (like ().__class__.__base__) can still be used to escape the sandbox.
**Prevention:** Implement an AST-based evaluator (SafeMathEvaluator) that strictly whitelists safe nodes (such as ast.BinOp, ast.Name, ast.Call) and explicitly blocks unsafe features like ast.Attribute.
