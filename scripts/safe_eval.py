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
            ast.BitAnd: operator.and_,
            ast.BitOr: operator.or_,
            ast.BitXor: operator.xor,
            ast.LShift: operator.lshift,
            ast.RShift: operator.rshift,

            ast.Eq: operator.eq,
            ast.NotEq: operator.ne,
            ast.Lt: operator.lt,
            ast.LtE: operator.le,
            ast.Gt: operator.gt,
            ast.GtE: operator.ge,
            ast.Is: operator.is_,
            ast.IsNot: operator.is_not,
        }

    def evaluate(self, expr_str):
        if not expr_str.strip():
            return None
        try:
            tree = ast.parse(expr_str, mode='eval')
            return self.visit(tree.body)
        except Exception as e:
            raise ValueError(f"Safe evaluation failed: {e}")

    def visit_Constant(self, node):
        return node.value

    # Fallbacks for Python < 3.8
    def visit_Num(self, node):
        return node.n

    def visit_Str(self, node):
        return node.s

    def visit_NameConstant(self, node):
        return node.value

    def visit_List(self, node):
        return [self.visit(elt) for elt in node.elts]

    def visit_Tuple(self, node):
        return tuple(self.visit(elt) for elt in node.elts)

    def visit_Dict(self, node):
        return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}

    def visit_Set(self, node):
        return {self.visit(elt) for elt in node.elts}

    def visit_UnaryOp(self, node):
        op_type = type(node.op)
        if op_type == ast.Not:
            return not self.visit(node.operand)
        elif op_type == ast.USub:
            return -self.visit(node.operand)
        elif op_type == ast.UAdd:
            return +self.visit(node.operand)
        elif op_type == ast.Invert:
            return ~self.visit(node.operand)
        raise ValueError(f"Unsupported unary operator: {op_type}")

    def visit_BinOp(self, node):
        op = self.operators.get(type(node.op))
        if op is None:
            raise ValueError(f"Unsupported binary operator: {type(node.op)}")
        return op(self.visit(node.left), self.visit(node.right))

    def visit_BoolOp(self, node):
        op_type = type(node.op)
        if op_type == ast.And:
            for value in node.values:
                result = self.visit(value)
                if not result:
                    return result
            return result
        elif op_type == ast.Or:
            for value in node.values:
                result = self.visit(value)
                if result:
                    return result
            return result
        raise ValueError(f"Unsupported boolean operator: {op_type}")

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op_node, comp_node in zip(node.ops, node.comparators):
            op_type = type(op_node)
            right = self.visit(comp_node)

            if op_type == ast.In:
                res = left in right
            elif op_type == ast.NotIn:
                res = left not in right
            else:
                op = self.operators.get(op_type)
                if op is None:
                    raise ValueError(f"Unsupported comparison operator: {op_type}")
                res = op(left, right)

            if not res:
                return False
            left = right
        return True

    def generic_visit(self, node):
        raise ValueError(f"Unsupported node type: {type(node).__name__}")

def safe_eval(expr):
    return SafeEvaluator().evaluate(expr)
