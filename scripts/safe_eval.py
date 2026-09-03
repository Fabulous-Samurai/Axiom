import ast
import operator
import math

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.operators = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv,
            ast.Mod: operator.mod,
            ast.Pow: operator.pow,
            ast.BitXor: operator.xor,
            ast.BitOr: operator.or_,
            ast.BitAnd: operator.and_,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos,
            ast.Eq: operator.eq,
            ast.NotEq: operator.ne,
            ast.Lt: operator.lt,
            ast.LtE: operator.le,
            ast.Gt: operator.gt,
            ast.GtE: operator.ge,
            ast.Is: operator.is_,
            ast.IsNot: operator.is_not,
            ast.In: lambda a, b: a in b,
            ast.NotIn: lambda a, b: a not in b,
        }
        self.functions = {
            'abs': abs,
            'min': min,
            'max': max,
            'round': round,
            'sin': math.sin,
            'cos': math.cos,
            'tan': math.tan,
            'sqrt': math.sqrt,
            'log': math.log,
            'log10': math.log10,
            'exp': math.exp,
        }

    def visit_Constant(self, node):
        return node.value

    def visit_NameConstant(self, node):
        return node.value

    def visit_Num(self, node):
        return node.n

    def visit_Str(self, node):
        return node.s

    def visit_List(self, node):
        return [self.visit(elt) for elt in node.elts]

    def visit_Tuple(self, node):
        return tuple(self.visit(elt) for elt in node.elts)

    def visit_Dict(self, node):
        return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}

    def visit_Name(self, node):
        if node.id in self.functions:
            return self.functions[node.id]
        elif node.id == 'True':
            return True
        elif node.id == 'False':
            return False
        elif node.id == 'None':
            return None
        raise ValueError(f"Unknown variable or function: {node.id}")

    def visit_Call(self, node):
        func = self.visit(node.func)
        if not callable(func):
            raise ValueError(f"Not a callable function: {node.func}")
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op = self.operators.get(type(node.op))
        if op is None:
            raise ValueError(f"Unsupported binary operator: {type(node.op)}")
        return op(left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op = self.operators.get(type(node.op))
        if op is None:
            raise ValueError(f"Unsupported unary operator: {type(node.op)}")
        return op(operand)

    def visit_BoolOp(self, node):
        op = type(node.op)
        if op == ast.And:
            for value in node.values:
                result = self.visit(value)
                if not result:
                    return result
            return result
        elif op == ast.Or:
            for value in node.values:
                result = self.visit(value)
                if result:
                    return result
            return result
        raise ValueError(f"Unsupported boolean operator: {op}")

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, right in zip(node.ops, node.comparators):
            op_func = self.operators.get(type(op))
            if op_func is None:
                raise ValueError(f"Unsupported comparison operator: {type(op)}")
            right_val = self.visit(right)
            if not op_func(left, right_val):
                return False
            left = right_val
        return True

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node type: {type(node).__name__}")

def safe_eval(expr_str):
    try:
        tree = ast.parse(expr_str, mode='eval')
        evaluator = SafeEvaluator()
        return evaluator.visit(tree.body)
    except SyntaxError as e:
        raise ValueError(f"Invalid syntax: {e}")
