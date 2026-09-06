import ast
import operator
import math

class SafeEval(ast.NodeVisitor):
    def __init__(self):
        self.allowed_functions = {
            'abs': abs, 'min': min, 'max': max, 'sum': sum, 'round': round,
            'len': len, 'int': int, 'float': float, 'str': str, 'bool': bool,
            'list': list, 'dict': dict, 'set': set, 'tuple': tuple, 'math': math
        }
        self.allowed_operators = {
            ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
            ast.Div: operator.truediv, ast.FloorDiv: operator.floordiv,
            ast.Mod: operator.mod, ast.Pow: operator.pow, ast.Eq: operator.eq,
            ast.NotEq: operator.ne, ast.Lt: operator.lt, ast.LtE: operator.le,
            ast.Gt: operator.gt, ast.GtE: operator.ge, ast.Is: operator.is_,
            ast.IsNot: operator.is_not, ast.In: operator.contains,
            ast.NotIn: lambda a, b: not operator.contains(b, a),
            ast.Not: operator.not_, ast.UAdd: operator.pos, ast.USub: operator.neg,
            ast.BitAnd: operator.and_, ast.BitOr: operator.or_,
            ast.BitXor: operator.xor, ast.Invert: operator.invert,
            ast.LShift: operator.lshift, ast.RShift: operator.rshift
        }

    def eval(self, expr):
        tree = ast.parse(expr, mode='eval')
        return self.visit(tree.body)

    def visit_Constant(self, node):
        return node.value

    def visit_List(self, node):
        return [self.visit(elt) for elt in node.elts]

    def visit_Tuple(self, node):
        return tuple(self.visit(elt) for elt in node.elts)

    def visit_Dict(self, node):
        return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}

    def visit_Set(self, node):
        return {self.visit(elt) for elt in node.elts}

    def visit_Name(self, node):
        if node.id in self.allowed_functions:
            return self.allowed_functions[node.id]
        raise ValueError(f"Name '{node.id}' is not allowed")

    def visit_UnaryOp(self, node):
        op = type(node.op)
        if op in self.allowed_operators:
            return self.allowed_operators[op](self.visit(node.operand))
        raise ValueError(f"Unary operator {op} not allowed")

    def visit_BinOp(self, node):
        op = type(node.op)
        if op in self.allowed_operators:
            return self.allowed_operators[op](self.visit(node.left), self.visit(node.right))
        raise ValueError(f"Binary operator {op} not allowed")

    def visit_BoolOp(self, node):
        op = type(node.op)
        if op == ast.And:
            result = True
            for value in node.values:
                result = result and self.visit(value)
                if not result:
                    break
            return result
        elif op == ast.Or:
            result = False
            for value in node.values:
                result = result or self.visit(value)
                if result:
                    break
            return result
        raise ValueError(f"Boolean operator {op} not allowed")

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, comparator in zip(node.ops, node.comparators):
            right = self.visit(comparator)
            op_type = type(op)
            if op_type == ast.In:
                if not operator.contains(right, left): return False
            elif op_type == ast.NotIn:
                if operator.contains(right, left): return False
            elif op_type in self.allowed_operators:
                if not self.allowed_operators[op_type](left, right): return False
            else:
                raise ValueError(f"Comparison operator {op_type} not allowed")
            left = right
        return True

    def visit_Call(self, node):
        func = self.visit(node.func)
        args = [self.visit(arg) for arg in node.args]
        kwargs = {kw.arg: self.visit(kw.value) for kw in node.keywords}
        if callable(func):
            return func(*args, **kwargs)
        raise ValueError("Function call not allowed on non-callable")

    def visit_Attribute(self, node):
        obj = self.visit(node.value)
        if isinstance(obj, (str, list, dict, set, tuple, int, float, bool)) or obj is math:
            if not node.attr.startswith('_'):
                return getattr(obj, node.attr)
        raise ValueError(f"Attribute access for {node.attr} not allowed")

    def visit_Subscript(self, node):
        obj = self.visit(node.value)
        if isinstance(node.slice, ast.Slice):
            lower = self.visit(node.slice.lower) if node.slice.lower else None
            upper = self.visit(node.slice.upper) if node.slice.upper else None
            step = self.visit(node.slice.step) if node.slice.step else None
            return obj[slice(lower, upper, step)]
        else:
            return obj[self.visit(node.slice)]

    def visit_IfExp(self, node):
        if self.visit(node.test):
            return self.visit(node.body)
        return self.visit(node.orelse)

    def generic_visit(self, node):
        raise ValueError(f"Node type {type(node).__name__} not allowed")

def safe_eval(expr):
    return SafeEval().eval(expr)
