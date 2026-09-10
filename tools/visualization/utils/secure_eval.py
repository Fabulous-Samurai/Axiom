import ast
import operator

class SecureEvaluator(ast.NodeVisitor):
    def __init__(self, safe_dict):
        self.safe_dict = safe_dict
        self.bin_ops = {
            ast.Add: operator.add, ast.Sub: operator.sub,
            ast.Mult: operator.mul, ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv, ast.Mod: operator.mod,
            ast.Pow: operator.pow, ast.BitAnd: operator.and_,
            ast.BitOr: operator.or_, ast.BitXor: operator.xor,
            ast.LShift: operator.lshift, ast.RShift: operator.rshift,
        }
        self.unary_ops = {
            ast.USub: operator.neg, ast.UAdd: operator.pos,
            ast.Not: operator.not_, ast.Invert: operator.invert
        }
        self.cmp_ops = {
            ast.Eq: operator.eq, ast.NotEq: operator.ne,
            ast.Lt: operator.lt, ast.LtE: operator.le,
            ast.Gt: operator.gt, ast.GtE: operator.ge,
            ast.Is: operator.is_, ast.IsNot: operator.is_not,
            ast.In: lambda a, b: a in b,
            ast.NotIn: lambda a, b: a not in b
        }
        self.bool_ops = {
            ast.And: lambda a, b: a and b,
            ast.Or: lambda a, b: a or b
        }

    def eval(self, expr):
        if not isinstance(expr, str):
            return expr
        if not expr.strip():
            return None
        node = ast.parse(expr, mode='eval')
        return self.visit(node.body)

    def visit_Constant(self, node):
        return node.value

    # Legacy support for Python < 3.8
    def visit_Num(self, node):
        return node.n

    def visit_Str(self, node):
        return node.s

    def visit_NameConstant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.safe_dict:
            return self.safe_dict[node.id]
        raise NameError(f"Name '{node.id}' is not defined")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op = type(node.op)
        if op in self.bin_ops:
            return self.bin_ops[op](left, right)
        raise ValueError(f"Unsupported binary operator: {op}")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op = type(node.op)
        if op in self.unary_ops:
            return self.unary_ops[op](operand)
        raise ValueError(f"Unsupported unary operator: {op}")

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op_node, comp_node in zip(node.ops, node.comparators):
            op = type(op_node)
            right = self.visit(comp_node)
            if op in self.cmp_ops:
                if not self.cmp_ops[op](left, right):
                    return False
                left = right
            else:
                raise ValueError(f"Unsupported comparison operator: {op}")
        return True

    def visit_BoolOp(self, node):
        values = [self.visit(v) for v in node.values]
        op = type(node.op)
        if op in self.bool_ops:
            result = values[0]
            for val in values[1:]:
                result = self.bool_ops[op](result, val)
            return result
        raise ValueError(f"Unsupported boolean operator: {op}")

    def visit_Call(self, node):
        func = self.visit(node.func)
        args = [self.visit(arg) for arg in node.args]
        kwargs = {kw.arg: self.visit(kw.value) for kw in node.keywords}
        if not callable(func):
            raise TypeError(f"'{type(func).__name__}' object is not callable")
        return func(*args, **kwargs)

    def visit_Tuple(self, node):
        return tuple(self.visit(elt) for elt in node.elts)

    def visit_List(self, node):
        return [self.visit(elt) for elt in node.elts]

    def visit_Dict(self, node):
        return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}

    def visit_Set(self, node):
        return {self.visit(elt) for elt in node.elts}

    def visit_Subscript(self, node):
        value = self.visit(node.value)
        slice_val = self.visit(node.slice)
        return value[slice_val]

    def visit_Slice(self, node):
        lower = self.visit(node.lower) if node.lower else None
        upper = self.visit(node.upper) if node.upper else None
        step = self.visit(node.step) if node.step else None
        return slice(lower, upper, step)

    def visit_Index(self, node):
        return self.visit(node.value)

    def visit_Attribute(self, node):
        value = self.visit(node.value)
        if node.attr.startswith('__'):
            raise ValueError(f"Access to private attribute '{node.attr}' is denied")
        if hasattr(value, node.attr):
            return getattr(value, node.attr)
        raise AttributeError(f"'{type(value).__name__}' object has no attribute '{node.attr}'")

    def visit_IfExp(self, node):
        test = self.visit(node.test)
        if test:
            return self.visit(node.body)
        else:
            return self.visit(node.orelse)

def secure_eval(expr, safe_dict=None):
    if safe_dict is None:
        safe_dict = {}
    evaluator = SecureEvaluator(safe_dict)
    return evaluator.eval(expr)
