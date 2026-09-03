import ast
import operator
import math

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.operators = {
            ast.Add: operator.add, ast.Sub: operator.sub,
            ast.Mult: operator.mul, ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv, ast.Mod: operator.mod,
            ast.Pow: operator.pow, ast.BitXor: operator.xor,
            ast.BitOr: operator.or_, ast.BitAnd: operator.and_,
            ast.USub: operator.neg, ast.UAdd: operator.pos,
            ast.Eq: operator.eq, ast.NotEq: operator.ne,
            ast.Lt: operator.lt, ast.LtE: operator.le,
            ast.Gt: operator.gt, ast.GtE: operator.ge,
            ast.Is: operator.is_, ast.IsNot: operator.is_not,
            ast.In: lambda a, b: a in b, ast.NotIn: lambda a, b: a not in b
        }
        self.funcs = {
            'abs': abs, 'min': min, 'max': max, 'round': round,
            'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
            'sqrt': math.sqrt, 'log': math.log, 'log10': math.log10, 'exp': math.exp
        }
        self.constants = {'True': True, 'False': False, 'None': None, 'pi': math.pi, 'e': math.e}

    def visit_Constant(self, node): return node.value
    def visit_NameConstant(self, node): return node.value
    def visit_Num(self, node): return node.n
    def visit_Str(self, node): return node.s

    def visit_List(self, node):
        if len(node.elts) > 1000: raise ValueError("List too large")
        return [self.visit(e) for e in node.elts]

    def visit_Tuple(self, node):
        if len(node.elts) > 1000: raise ValueError("Tuple too large")
        return tuple(self.visit(e) for e in node.elts)

    def visit_Dict(self, node):
        if len(node.keys) > 1000: raise ValueError("Dict too large")
        return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}

    def visit_Name(self, node):
        if node.id in self.funcs: return self.funcs[node.id]
        if node.id in self.constants: return self.constants[node.id]
        raise ValueError(f"Unknown variable or function: {node.id}")

    def visit_Attribute(self, node):
        val = self.visit(node.value)
        if hasattr(val, node.attr) and node.attr in self.funcs: return getattr(val, node.attr)
        raise ValueError(f"Unknown attribute: {node.attr}")

    def visit_Subscript(self, node):
        return self.visit(node.value)[self.visit(node.slice)]

    def visit_Slice(self, node):
        return slice(self.visit(node.lower) if node.lower else None,
                     self.visit(node.upper) if node.upper else None,
                     self.visit(node.step) if node.step else None)

    def visit_Index(self, node):
        return self.visit(node.value)

    def visit_Call(self, node):
        func = self.visit(node.func)
        args = [self.visit(a) for a in node.args]
        kwargs = {kw.arg: self.visit(kw.value) for kw in node.keywords}
        return func(*args, **kwargs)

    def visit_BinOp(self, node):
        left, right = self.visit(node.left), self.visit(node.right)
        op = self.operators.get(type(node.op))
        if type(node.op) == ast.Pow and (right > 100 or right < -100):
            raise ValueError("Exponent too large")
        if type(node.op) == ast.Mult and (isinstance(left, (str, list, tuple)) or isinstance(right, (str, list, tuple))):
            if (isinstance(left, int) and left > 1000) or (isinstance(right, int) and right > 1000):
                 raise ValueError("Multiplication limit exceeded")
        return op(left, right)

    def visit_UnaryOp(self, node):
        return self.operators.get(type(node.op))(self.visit(node.operand))

    def visit_BoolOp(self, node):
        op = type(node.op)
        if op == ast.And: return all(self.visit(v) for v in node.values)
        if op == ast.Or: return any(self.visit(v) for v in node.values)

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, right in zip(node.ops, node.comparators):
            op_func, right_val = self.operators.get(type(op)), self.visit(right)
            if not op_func(left, right_val): return False
            left = right_val
        return True

    def visit_IfExp(self, node):
        return self.visit(node.body) if self.visit(node.test) else self.visit(node.orelse)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node type: {type(node).__name__}")

def safe_eval(expr_str):
    try:
        return SafeEvaluator().visit(ast.parse(expr_str, mode='eval').body)
    except SyntaxError as e:
        raise ValueError(f"Invalid syntax: {e}")
