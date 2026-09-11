import ast
import operator

class SafeEvaluator:
    def __init__(self, allowed_names):
        self.allowed_names = allowed_names
        self.bin_ops = {
            ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
            ast.Div: operator.truediv, ast.Pow: operator.pow, ast.Mod: operator.mod,
            ast.BitXor: operator.xor, ast.BitOr: operator.or_, ast.BitAnd: operator.and_,
            ast.FloorDiv: operator.floordiv, ast.LShift: operator.lshift, ast.RShift: operator.rshift
        }
        self.un_ops = {
            ast.USub: operator.neg, ast.UAdd: operator.pos,
            ast.Not: operator.not_, ast.Invert: operator.invert
        }
        self.cmp_ops = {
            ast.Eq: operator.eq, ast.NotEq: operator.ne, ast.Lt: operator.lt,
            ast.LtE: operator.le, ast.Gt: operator.gt, ast.GtE: operator.ge,
            ast.Is: operator.is_, ast.IsNot: operator.is_not,
            ast.In: lambda a, b: a in b, ast.NotIn: lambda a, b: a not in b
        }

    def evaluate(self, expr):
        try:
            tree = ast.parse(expr, mode='eval')
            return self._eval_node(tree.body)
        except Exception as e:
            raise ValueError(f"Invalid expression: {e}")

    def _eval_node(self, node):
        if isinstance(node, ast.Constant):
            return node.value
        elif isinstance(node, ast.Name):
            if node.id in self.allowed_names:
                return self.allowed_names[node.id]
            raise ValueError(f"Name '{node.id}' not allowed")
        elif isinstance(node, ast.BinOp):
            return self.bin_ops[type(node.op)](self._eval_node(node.left), self._eval_node(node.right))
        elif isinstance(node, ast.UnaryOp):
            return self.un_ops[type(node.op)](self._eval_node(node.operand))
        elif isinstance(node, ast.Call):
            func = self._eval_node(node.func)
            args = [self._eval_node(arg) for arg in node.args]
            kwargs = {kw.arg: self._eval_node(kw.value) for kw in node.keywords}
            return func(*args, **kwargs)
        elif isinstance(node, ast.Compare):
            left = self._eval_node(node.left)
            for op, comp in zip(node.ops, node.comparators):
                right = self._eval_node(comp)
                if not self.cmp_ops[type(op)](left, right):
                    return False
                left = right
            return True
        elif isinstance(node, ast.BoolOp):
            if isinstance(node.op, ast.And):
                return all(self._eval_node(v) for v in node.values)
            return any(self._eval_node(v) for v in node.values)
        elif isinstance(node, ast.Subscript):
            return self._eval_node(node.value)[self._eval_node(node.slice)]
        elif isinstance(node, ast.Attribute):
            obj = self._eval_node(node.value)
            if hasattr(obj, node.attr) and not node.attr.startswith('_'):
                return getattr(obj, node.attr)
            raise ValueError(f"Attribute '{node.attr}' not allowed")
        elif isinstance(node, ast.IfExp):
            return self._eval_node(node.body) if self._eval_node(node.test) else self._eval_node(node.orelse)
        elif isinstance(node, ast.Tuple):
            return tuple(self._eval_node(elt) for elt in node.elts)
        elif isinstance(node, ast.List):
            return [self._eval_node(elt) for elt in node.elts]
        raise ValueError(f"Unsupported node type: {type(node).__name__}")
