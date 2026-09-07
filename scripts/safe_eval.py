import ast
import operator
import math

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self, variables=None, functions=None):
        self.variables = variables or {}
        self.functions = functions or {'sin': math.sin, 'cos': math.cos, 'tan': math.tan, 'log': math.log, 'sqrt': math.sqrt, 'abs': abs, 'min': min, 'max': max, 'round': round}

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.variables:
            return self.variables[node.id]
        if node.id in self.functions:
            return self.functions[node.id]
        raise NameError(f"Name '{node.id}' is not allowed")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        ops = {
            ast.Add: operator.add, ast.Sub: operator.sub,
            ast.Mult: operator.mul, ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv, ast.Mod: operator.mod,
            ast.Pow: operator.pow, ast.BitAnd: operator.and_,
            ast.BitOr: operator.or_, ast.BitXor: operator.xor,
        }
        return ops[type(node.op)](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        ops = {
            ast.UAdd: operator.pos, ast.USub: operator.neg,
            ast.Not: operator.not_, ast.Invert: operator.invert,
        }
        return ops[type(node.op)](operand)

    def visit_BoolOp(self, node):
        if isinstance(node.op, ast.And):
            res = True
            for val in node.values:
                res = self.visit(val)
                if not res: return res
            return res
        elif isinstance(node.op, ast.Or):
            res = False
            for val in node.values:
                res = self.visit(val)
                if res: return res
            return res
        raise ValueError(f"Unsupported BoolOp {type(node.op)}")

    def visit_Compare(self, node):
        left = self.visit(node.left)
        ops = {
            ast.Eq: operator.eq, ast.NotEq: operator.ne,
            ast.Lt: operator.lt, ast.LtE: operator.le,
            ast.Gt: operator.gt, ast.GtE: operator.ge,
            ast.In: lambda a, b: a in b, ast.NotIn: lambda a, b: a not in b,
        }
        for op, comp in zip(node.ops, node.comparators):
            right = self.visit(comp)
            if not ops[type(op)](left, right):
                return False
            left = right
        return True

    def visit_Call(self, node):
        func = self.visit(node.func)
        args = [self.visit(arg) for arg in node.args]
        kwargs = {kw.arg: self.visit(kw.value) for kw in node.keywords}
        return func(*args, **kwargs)

    def visit_Attribute(self, node):
        value = self.visit(node.value)
        if node.attr.startswith('_'):
            raise AttributeError(f"Access to private attribute '{node.attr}' is not allowed")
        return getattr(value, node.attr)

    def visit_Subscript(self, node):
        value = self.visit(node.value)
        slice_val = self.visit(node.slice)
        return value[slice_val]

    def visit_Slice(self, node):
        lower = self.visit(node.lower) if node.lower else None
        upper = self.visit(node.upper) if node.upper else None
        step = self.visit(node.step) if node.step else None
        return slice(lower, upper, step)

    def visit_IfExp(self, node):
        return self.visit(node.body) if self.visit(node.test) else self.visit(node.orelse)

    def visit_Tuple(self, node):
        return tuple(self.visit(e) for e in node.elts)

    def visit_List(self, node):
        return [self.visit(e) for e in node.elts]

    def visit_Dict(self, node):
        return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}

    def generic_visit(self, node):
        raise ValueError(f"Unsupported operation: {type(node).__name__}")

def evaluate(expr, variables=None, functions=None):
    tree = ast.parse(expr, mode='eval')
    return SafeEvaluator(variables, functions).visit(tree)
