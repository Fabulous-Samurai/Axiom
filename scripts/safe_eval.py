import ast
import operator

class SafeEvaluator(ast.NodeVisitor):
    allowed_ops = {
        ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
        ast.Div: operator.truediv, ast.FloorDiv: operator.floordiv,
        ast.Mod: operator.mod, ast.Pow: operator.pow, ast.BitXor: operator.xor,
        ast.BitOr: operator.or_, ast.BitAnd: operator.and_,
        ast.LShift: operator.lshift, ast.RShift: operator.rshift,
        ast.Eq: operator.eq, ast.NotEq: operator.ne, ast.Lt: operator.lt,
        ast.LtE: operator.le, ast.Gt: operator.gt, ast.GtE: operator.ge,
        ast.Is: operator.is_, ast.IsNot: operator.is_not,
        ast.In: lambda a, b: a in b, ast.NotIn: lambda a, b: a not in b,
        ast.UAdd: operator.pos, ast.USub: operator.neg,
        ast.Not: operator.not_, ast.Invert: operator.invert
    }

    def visit_Constant(self, node): return node.value
    def visit_List(self, node): return [self.visit(elt) for elt in node.elts]
    def visit_Tuple(self, node): return tuple(self.visit(elt) for elt in node.elts)
    def visit_Set(self, node): return {self.visit(elt) for elt in node.elts}
    def visit_Dict(self, node): return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}

    def visit_UnaryOp(self, node):
        return self.allowed_ops[type(node.op)](self.visit(node.operand))

    def visit_BinOp(self, node):
        return self.allowed_ops[type(node.op)](self.visit(node.left), self.visit(node.right))

    def visit_BoolOp(self, node):
        values = [self.visit(v) for v in node.values]
        if isinstance(node.op, ast.And): return all(values)
        return any(values)

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op_node, comparator_node in zip(node.ops, node.comparators):
            right = self.visit(comparator_node)
            if not self.allowed_ops[type(op_node)](left, right):
                return False
            left = right
        return True

    def visit_Expression(self, node): return self.visit(node.body)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported syntax: {type(node).__name__}")

def evaluate(expr_str):
    return SafeEvaluator().visit(ast.parse(expr_str, mode='eval'))
