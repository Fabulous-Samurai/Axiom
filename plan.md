1. **Create `SafeMathEvaluator` in `tools/visualization/advanced_3d_visualization.py`**
   - Use `replace_with_git_merge_diff` to add `import ast` and `import operator` and the `SafeMathEvaluator` class under the existing imports.

```
<<<<<<< SEARCH
import time

# Constants for default function strings
=======
import time
import ast
import operator

class SafeMathEvaluator:
    def __init__(self, safe_dict): self.safe_dict = safe_dict
    def evaluate(self, expr): return self._eval(ast.parse(expr, mode='eval').body)
    def _eval(self, node):
        if isinstance(node, ast.Constant): return node.value
        if isinstance(node, ast.Name): return self.safe_dict[node.id]
        if isinstance(node, ast.BinOp):
            ops = {ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul, ast.Div: operator.truediv, ast.Pow: operator.pow}
            return ops[type(node.op)](self._eval(node.left), self._eval(node.right))
        if isinstance(node, ast.UnaryOp):
            return {ast.UAdd: operator.pos, ast.USub: operator.neg}[type(node.op)](self._eval(node.operand))
        if isinstance(node, ast.Call): return self._eval(node.func)(*[self._eval(a) for a in node.args])
        if isinstance(node, ast.Tuple): return tuple(self._eval(e) for e in node.elts)
        if isinstance(node, ast.List): return list(self._eval(e) for e in node.elts)
        if isinstance(node, ast.Load): return None
        if isinstance(node, ast.Attribute): raise ValueError("Attribute access disallowed")
        raise ValueError("Unsupported")

# Constants for default function strings
>>>>>>> REPLACE
```

2. **Replace `eval()` with `SafeMathEvaluator` in `tools/visualization/advanced_3d_visualization.py`**
   - Use `replace_with_git_merge_diff` multiple times to replace all `eval` calls.

```
<<<<<<< SEARCH
            Z = eval(func_str, {"__builtins__": {}}, safe_dict)
=======
            Z = SafeMathEvaluator(safe_dict).evaluate(func_str)
>>>>>>> REPLACE
```
and
```
<<<<<<< SEARCH
            x = eval(x_func, {"__builtins__": {}}, safe_dict)
            y = eval(y_func, {"__builtins__": {}}, safe_dict)
            z = eval(z_func, {"__builtins__": {}}, safe_dict)
=======
            x = SafeMathEvaluator(safe_dict).evaluate(x_func)
            y = SafeMathEvaluator(safe_dict).evaluate(y_func)
            z = SafeMathEvaluator(safe_dict).evaluate(z_func)
>>>>>>> REPLACE
```
and
```
<<<<<<< SEARCH
            z_base = eval(base_func, {"__builtins__": {}}, safe_dict)
            time_mod = eval(time_modulation, {"__builtins__": {}}, safe_dict)
=======
            z_base = SafeMathEvaluator(safe_dict).evaluate(base_func)
            time_mod = SafeMathEvaluator(safe_dict).evaluate(time_modulation)
>>>>>>> REPLACE
```
and
```
<<<<<<< SEARCH
                z_base = eval(base_func, {"__builtins__": {}}, safe_dict)
                time_mod = eval(time_modulation, {"__builtins__": {}}, safe_dict)
=======
                z_base = SafeMathEvaluator(safe_dict).evaluate(base_func)
                time_mod = SafeMathEvaluator(safe_dict).evaluate(time_modulation)
>>>>>>> REPLACE
```
and
```
<<<<<<< SEARCH
                Z = amplitude * eval(func_str, {"__builtins__": {}}, safe_dict)
=======
                Z = amplitude * SafeMathEvaluator(safe_dict).evaluate(func_str)
>>>>>>> REPLACE
```
and
```
<<<<<<< SEARCH
                t_min, t_max = eval(f"({t_range_str})", {"pi": np.pi})
=======
                t_min, t_max = SafeMathEvaluator({"pi": np.pi}).evaluate(f"({t_range_str})")
>>>>>>> REPLACE
```

3. **Verify Modifications**
   - Run `git diff tools/visualization/advanced_3d_visualization.py` to confirm changes look correct.

4. **Log the Finding in `.jules/sentinel.md`**
   - Use bash to append the vulnerability journal entry:
```bash
mkdir -p .jules && cat << 'EOF' >> .jules/sentinel.md
## 2024-03-22 - [CRITICAL] Prevent Command Injection via eval() in 3D Visualization
**Vulnerability:** Unsafe usage of eval() in tools/visualization/advanced_3d_visualization.py allowed arbitrary Python code execution (e.g., __import__('os')) despite passing an empty __builtins__ dictionary.
**Learning:** Overriding __builtins__ to empty does not fully secure eval(), as object introspection (like ().__class__.__base__) can still be used to escape the sandbox.
**Prevention:** Implement an AST-based evaluator (SafeMathEvaluator) that strictly whitelists safe nodes (such as ast.BinOp, ast.Name, ast.Call) and explicitly blocks unsafe features like ast.Attribute.
EOF
```
   - Run `cat .jules/sentinel.md` to confirm its contents.

5. **Verify Tests**
   - Run Python tests: `PYTHONPATH=.:products/gui:products pytest tests/sandbox_security_test.py`
   - Run `python3 -m py_compile tools/visualization/advanced_3d_visualization.py` to check for syntax errors.

6. **Complete pre-commit steps**
   - Complete pre-commit steps to ensure proper testing, verification, review, and reflection are done.

7. **Submit PR**
   - Clean up workspace: `rm plan.md`
   - Call `submit` tool with `branch_name` = `sentinel-fix-eval-rce-v2`, `commit_message` = `🛡️ Sentinel: [CRITICAL] Fix command injection vulnerability in 3D visualization`, `title` = `🛡️ Sentinel: [CRITICAL] Fix command injection in 3D visualization eval`, and `description` matching the PR required format.
