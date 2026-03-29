## 2025-02-17 - [Safe Math Evaluation]
**Vulnerability:** Use of `eval()` in `tools/visualization/advanced_3d_visualization.py` allowed arbitrary code execution due to empty `__builtins__` override, as users could inject arbitrary python execution via UI or parameters.
**Learning:** `eval(expr, {"__builtins__": {}})` is not safe; it's easily bypassable. Sandbox environments require rigorous AST validation instead of dictionary injection overrides.
**Prevention:** Implement and reuse `SafeMathEvaluator` leveraging `ast.parse` and whitelist-based execution nodes (`_eval_node`), entirely removing unsafe operators (e.g., `ast.Attribute`).
