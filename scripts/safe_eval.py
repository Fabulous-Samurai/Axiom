import ast
import operator

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
            ast.LShift: operator.lshift,
            ast.RShift: operator.rshift,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos,
            ast.Not: operator.not_,
            ast.Invert: operator.invert,
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

    def visit_Constant(self, node):
        return node.value

    def visit_List(self, node):
        return [self.visit(elt) for elt in node.elts]

    def visit_Tuple(self, node):
        return tuple(self.visit(elt) for elt in node.elts)

    def visit_Set(self, node):
        return {self.visit(elt) for elt in node.elts}

    def visit_Dict(self, node):
        return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}

    def visit_UnaryOp(self, node):
        op = self.operators[type(node.op)]
        return op(self.visit(node.operand))

    def visit_BinOp(self, node):
        op = self.operators[type(node.op)]
        return op(self.visit(node.left), self.visit(node.right))

    def visit_BoolOp(self, node):
        if isinstance(node.op, ast.And):
            for value in node.values:
                if not self.visit(value):
                    return False
            return True
        elif isinstance(node.op, ast.Or):
            for value in node.values:
                if self.visit(value):
                    return True
            return False

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, comparator in zip(node.ops, node.comparators):
            right = self.visit(comparator)
            if not self.operators[type(op)](left, right):
                return False
            left = right
        return True

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

def evaluate(expression):
    node = ast.parse(expression, mode='eval')
    return SafeEvaluator().visit(node.body)
