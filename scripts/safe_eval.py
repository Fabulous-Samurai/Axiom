import ast
import math

SAFE_FUNCTIONS = {
    'abs': abs,
    'min': min,
    'max': max,
    'round': round,
    'sum': sum,
    'sin': math.sin,
    'cos': math.cos,
    'tan': math.tan,
    'sqrt': math.sqrt,
    'log': math.log,
    'exp': math.exp,
}

class SafeEvaluator(ast.NodeVisitor):
    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in SAFE_FUNCTIONS:
            return SAFE_FUNCTIONS[node.id]
        if node.id == 'True':
            return True
        if node.id == 'False':
            return False
        if node.id == 'None':
            return None
        if node.id == 'pi':
            return math.pi
        if node.id == 'e':
            return math.e
        raise ValueError(f"Unsupported variable or function: {node.id}")

    def visit_UnaryOp(self, node):
        op = node.op
        operand = self.visit(node.operand)
        if isinstance(op, ast.UAdd):
            return +operand
        elif isinstance(op, ast.USub):
            return -operand
        elif isinstance(op, ast.Not):
            return not operand
        elif isinstance(op, ast.Invert):
            return ~operand
        raise ValueError(f"Unsupported unary operator: {type(op).__name__}")

    def visit_BinOp(self, node):
        op = node.op
        left = self.visit(node.left)
        right = self.visit(node.right)
        if isinstance(op, ast.Add):
            return left + right
        elif isinstance(op, ast.Sub):
            return left - right
        elif isinstance(op, ast.Mult):
            return left * right
        elif isinstance(op, ast.Div):
            return left / right
        elif isinstance(op, ast.FloorDiv):
            return left // right
        elif isinstance(op, ast.Mod):
            return left % right
        elif isinstance(op, ast.Pow):
            return left ** right
        elif isinstance(op, ast.BitAnd):
            return left & right
        elif isinstance(op, ast.BitOr):
            return left | right
        elif isinstance(op, ast.BitXor):
            return left ^ right
        elif isinstance(op, ast.LShift):
            return left << right
        elif isinstance(op, ast.RShift):
            return left >> right
        raise ValueError(f"Unsupported binary operator: {type(op).__name__}")

    def visit_BoolOp(self, node):
        values = [self.visit(v) for v in node.values]
        if isinstance(node.op, ast.And):
            return all(values)
        elif isinstance(node.op, ast.Or):
            return any(values)
        raise ValueError(f"Unsupported boolean operator: {type(node.op).__name__}")

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, comparator in zip(node.ops, node.comparators):
            right = self.visit(comparator)
            if isinstance(op, ast.Eq):
                res = left == right
            elif isinstance(op, ast.NotEq):
                res = left != right
            elif isinstance(op, ast.Lt):
                res = left < right
            elif isinstance(op, ast.LtE):
                res = left <= right
            elif isinstance(op, ast.Gt):
                res = left > right
            elif isinstance(op, ast.GtE):
                res = left >= right
            elif isinstance(op, ast.In):
                res = left in right
            elif isinstance(op, ast.NotIn):
                res = left not in right
            elif isinstance(op, ast.Is):
                res = left is right
            elif isinstance(op, ast.IsNot):
                res = left is not right
            else:
                raise ValueError(f"Unsupported comparison operator: {type(op).__name__}")
            if not res:
                return False
            left = right
        return True

    def visit_Call(self, node):
        func = self.visit(node.func)
        if not callable(func):
            raise ValueError(f"Not a callable function")
        args = [self.visit(a) for a in node.args]
        kwargs = {k.arg: self.visit(k.value) for k in node.keywords if k.arg is not None}
        return func(*args, **kwargs)

    def visit_List(self, node):
        return [self.visit(elt) for elt in node.elts]

    def visit_Tuple(self, node):
        return tuple(self.visit(elt) for elt in node.elts)

    def visit_Set(self, node):
        return {self.visit(elt) for elt in node.elts}

    def visit_Dict(self, node):
        return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}

    def visit_Subscript(self, node):
        value = self.visit(node.value)
        slice_val = self.visit(node.slice)
        return value[slice_val]

    def visit_Attribute(self, node):
        value = self.visit(node.value)
        if node.attr.startswith('_'):
            raise ValueError("Access to private attributes is blocked")
        return getattr(value, node.attr)

    def visit_IfExp(self, node):
        test = self.visit(node.test)
        if test:
            return self.visit(node.body)
        else:
            return self.visit(node.orelse)

    def visit_Slice(self, node):
        lower = self.visit(node.lower) if node.lower else None
        upper = self.visit(node.upper) if node.upper else None
        step = self.visit(node.step) if node.step else None
        return slice(lower, upper, step)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

def safe_eval(expr):
    tree = ast.parse(expr, mode='eval')
    evaluator = SafeEvaluator()
    return evaluator.visit(tree.body)
