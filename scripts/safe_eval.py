import ast
import operator

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.ops = {
            ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
            ast.Div: operator.truediv, ast.FloorDiv: operator.floordiv,
            ast.Mod: operator.mod, ast.Pow: operator.pow, ast.USub: operator.neg,
            ast.UAdd: operator.pos, ast.Eq: operator.eq, ast.NotEq: operator.ne,
            ast.Lt: operator.lt, ast.LtE: operator.le, ast.Gt: operator.gt,
            ast.GtE: operator.ge, ast.And: lambda a, b: a and b,
            ast.Or: lambda a, b: a or b, ast.Not: operator.not_,
            ast.BitAnd: operator.and_, ast.BitOr: operator.or_, ast.BitXor: operator.xor,
            ast.Invert: operator.invert, ast.LShift: operator.lshift, ast.RShift: operator.rshift
        }

    def visit_Constant(self, node): return node.value
    def visit_List(self, node): return [self.visit(e) for e in node.elts]
    def visit_Dict(self, node): return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}
    def visit_Tuple(self, node): return tuple(self.visit(e) for e in node.elts)
    def visit_Set(self, node): return {self.visit(e) for e in node.elts}
    def visit_BinOp(self, node): return self.ops[type(node.op)](self.visit(node.left), self.visit(node.right))
    def visit_UnaryOp(self, node): return self.ops[type(node.op)](self.visit(node.operand))
    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, comp in zip(node.ops, node.comparators):
            right = self.visit(comp)
            if not self.ops[type(op)](left, right): return False
            left = right
        return True
    def visit_BoolOp(self, node):
        if isinstance(node.op, ast.And): return all(self.visit(v) for v in node.values)
        return any(self.visit(v) for v in node.values)
    def generic_visit(self, node): raise ValueError(f"Unsupported operation: {type(node).__name__}")

def evaluate(expr):
    node = ast.parse(expr, mode='eval').body
    return SafeEvaluator().visit(node)
