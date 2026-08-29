import ast
import operator

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_ops = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.Mod: operator.mod,
            ast.Pow: operator.pow,
            ast.BitXor: operator.xor,
            ast.USub: operator.neg,
            ast.Eq: operator.eq,
            ast.NotEq: operator.ne,
            ast.Lt: operator.lt,
            ast.LtE: operator.le,
            ast.Gt: operator.gt,
            ast.GtE: operator.ge,
            ast.Invert: operator.invert,
        }

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        # We don't allow variables, but might allow basic builtin names if required
        raise ValueError(f"Variables and functions are not allowed: {node.id}")

    def visit_List(self, node):
        return [self.visit(elt) for elt in node.elts]

    def visit_Dict(self, node):
        return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}

    def visit_Tuple(self, node):
        return tuple(self.visit(elt) for elt in node.elts)

    def visit_Set(self, node):
        return set(self.visit(elt) for elt in node.elts)

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op_type = type(node.op)
        if op_type in self.allowed_ops:
            return self.allowed_ops[op_type](left, right)
        raise ValueError(f"Unsupported binary operator: {op_type.__name__}")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op_type = type(node.op)
        if op_type in self.allowed_ops:
            return self.allowed_ops[op_type](operand)
        raise ValueError(f"Unsupported unary operator: {op_type.__name__}")

    def visit_BoolOp(self, node):
        op_type = type(node.op)
        if op_type == ast.And:
            for value in node.values:
                if not self.visit(value):
                    return False
            return True
        elif op_type == ast.Or:
            for value in node.values:
                if self.visit(value):
                    return True
            return False
        raise ValueError(f"Unsupported boolean operator: {op_type.__name__}")

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, comparator in zip(node.ops, node.comparators):
            right = self.visit(comparator)
            op_type = type(op)
            if op_type in self.allowed_ops:
                if not self.allowed_ops[op_type](left, right):
                    return False
                left = right
            else:
                raise ValueError(f"Unsupported comparison operator: {op_type.__name__}")
        return True

    def visit_Expression(self, node):
        return self.visit(node.body)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported syntax: {type(node).__name__}")

def safe_eval(expr):
    if not isinstance(expr, str):
        return expr
    try:
        tree = ast.parse(expr, mode='eval')
        evaluator = SafeEvaluator()
        return evaluator.visit(tree)
    except Exception as e:
        raise ValueError(f"Unsafe or invalid expression: {e}")
