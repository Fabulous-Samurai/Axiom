import ast
import operator

class SafeEvaluator(ast.NodeVisitor):
    """
    Safely evaluates basic arithmetic, comparisons, booleans, and data structures.
    Prevents execution of arbitrary Python functions.
    """
    def __init__(self):
        self.allowed_operators = {
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
            ast.In: operator.contains,
        }

    def evaluate(self, expr):
        try:
            node = ast.parse(expr, mode='eval')
            return self.visit(node.body)
        except Exception as e:
            raise ValueError(f"Invalid expression: {e}")

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

    def visit_Set(self, node):
        return {self.visit(elt) for elt in node.elts}

    def visit_UnaryOp(self, node):
        op = type(node.op)
        if op not in self.allowed_operators:
            raise ValueError(f"Unsupported operator: {op}")
        return self.allowed_operators[op](self.visit(node.operand))

    def visit_BinOp(self, node):
        op = type(node.op)
        if op not in self.allowed_operators:
            raise ValueError(f"Unsupported operator: {op}")
        return self.allowed_operators[op](self.visit(node.left), self.visit(node.right))

    def visit_BoolOp(self, node):
        op = type(node.op)
        if op == ast.And:
            result = None
            for value in node.values:
                result = self.visit(value)
                if not result:
                    return result
            return result
        elif op == ast.Or:
            result = None
            for value in node.values:
                result = self.visit(value)
                if result:
                    return result
            return result
        raise ValueError(f"Unsupported bool operator: {op}")

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, comparator in zip(node.ops, node.comparators):
            op_type = type(op)
            right = self.visit(comparator)
            if op_type == ast.NotIn:
                if left in right:
                    return False
            else:
                if op_type not in self.allowed_operators:
                    raise ValueError(f"Unsupported compare operator: {op_type}")
                func = self.allowed_operators[op_type]
                if not (func(right, left) if op_type == ast.In else func(left, right)):
                    return False
            left = right
        return True

    def generic_visit(self, node):
        raise ValueError(f"Unsupported node type: {type(node)}")

def safe_eval(expr):
    evaluator = SafeEvaluator()
    return evaluator.evaluate(expr)
