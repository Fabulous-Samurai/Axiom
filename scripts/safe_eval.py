import ast
import operator

class SafeEval(ast.NodeVisitor):
    def __init__(self, allowed_names=None):
        self.allowed_names = allowed_names or {}

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.allowed_names:
            return self.allowed_names[node.id]
        raise ValueError(f"Name '{node.id}' is not allowed")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        if isinstance(node.op, ast.UAdd): return operator.pos(operand)
        elif isinstance(node.op, ast.USub): return operator.neg(operand)
        elif isinstance(node.op, ast.Not): return operator.not_(operand)
        raise ValueError(f"Unary operator '{type(node.op).__name__}' is not allowed")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if isinstance(node.op, ast.Add): return operator.add(left, right)
        elif isinstance(node.op, ast.Sub): return operator.sub(left, right)
        elif isinstance(node.op, ast.Mult): return operator.mul(left, right)
        elif isinstance(node.op, ast.Div): return operator.truediv(left, right)
        elif isinstance(node.op, ast.FloorDiv): return operator.floordiv(left, right)
        elif isinstance(node.op, ast.Mod): return operator.mod(left, right)
        elif isinstance(node.op, ast.Pow): return operator.pow(left, right)
        raise ValueError(f"Binary operator '{type(node.op).__name__}' is not allowed")

    def visit_BoolOp(self, node):
        if isinstance(node.op, ast.And):
            for value in node.values:
                if not self.visit(value): return False
            return True
        elif isinstance(node.op, ast.Or):
            for value in node.values:
                if self.visit(value): return True
            return False
        raise ValueError(f"Boolean operator '{type(node.op).__name__}' is not allowed")

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, right in zip(node.ops, node.comparators):
            right_val = self.visit(right)
            if isinstance(op, ast.Eq): res = operator.eq(left, right_val)
            elif isinstance(op, ast.NotEq): res = operator.ne(left, right_val)
            elif isinstance(op, ast.Lt): res = operator.lt(left, right_val)
            elif isinstance(op, ast.LtE): res = operator.le(left, right_val)
            elif isinstance(op, ast.Gt): res = operator.gt(left, right_val)
            elif isinstance(op, ast.GtE): res = operator.ge(left, right_val)
            elif isinstance(op, ast.In): res = operator.contains(right_val, left)
            elif isinstance(op, ast.NotIn): res = not operator.contains(right_val, left)
            else: raise ValueError(f"Comparison operator '{type(op).__name__}' is not allowed")
            if not res:
                return False
            left = right_val
        return True

    def visit_Call(self, node):
        func = self.visit(node.func)
        args = [self.visit(arg) for arg in node.args]
        kwargs = {kw.arg: self.visit(kw.value) for kw in node.keywords}
        return func(*args, **kwargs)

    def visit_Subscript(self, node):
        value = self.visit(node.value)
        slice_val = self.visit(node.slice)
        return value[slice_val]

    def visit_Slice(self, node):
        lower = self.visit(node.lower) if node.lower else None
        upper = self.visit(node.upper) if node.upper else None
        step = self.visit(node.step) if node.step else None
        return slice(lower, upper, step)

    def visit_Attribute(self, node):
        value = self.visit(node.value)
        if node.attr.startswith('_'):
            raise ValueError(f"Access to private attribute '{node.attr}' is not allowed")
        return getattr(value, node.attr)

    def visit_IfExp(self, node):
        test = self.visit(node.test)
        if test:
            return self.visit(node.body)
        else:
            return self.visit(node.orelse)

    def visit_List(self, node):
        return [self.visit(item) for item in node.elts]

    def visit_Tuple(self, node):
        return tuple(self.visit(item) for item in node.elts)

    def visit_Dict(self, node):
        return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}

    def generic_visit(self, node):
        raise ValueError(f"Unsupported expression node: {type(node).__name__}")

def safe_evaluate(expr, allowed_names=None):
    tree = ast.parse(expr, mode='eval')
    evaluator = SafeEval(allowed_names)
    return evaluator.visit(tree.body)
