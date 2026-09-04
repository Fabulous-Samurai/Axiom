import ast
import operator

class SafeEvaluator(ast.NodeVisitor):
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
            ast.UAdd: operator.pos,
            ast.USub: operator.neg,
            ast.Eq: operator.eq,
            ast.NotEq: operator.ne,
            ast.Lt: operator.lt,
            ast.LtE: operator.le,
            ast.Gt: operator.gt,
            ast.GtE: operator.ge,
            ast.In: lambda a, b: a in b,
            ast.NotIn: lambda a, b: a not in b,
            ast.Not: operator.not_,
        }
        self.allowed_functions = {
            'abs': abs,
            'round': round,
            'min': min,
            'max': max,
            'sum': sum,
            'len': len,
        }

    def visit_Constant(self, node):
        return node.value

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op_type = type(node.op)
        if op_type in self.allowed_operators:
            return self.allowed_operators[op_type](left, right)
        raise ValueError(f"Unsupported operator: {op_type}")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op_type = type(node.op)
        if op_type in self.allowed_operators:
            return self.allowed_operators[op_type](operand)
        raise ValueError(f"Unsupported operator: {op_type}")

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, comp in zip(node.ops, node.comparators):
            op_type = type(op)
            if op_type not in self.allowed_operators:
                raise ValueError(f"Unsupported comparison: {op_type}")
            right = self.visit(comp)
            if not self.allowed_operators[op_type](left, right):
                return False
            left = right
        return True

    def visit_BoolOp(self, node):
        values = [self.visit(val) for val in node.values]
        if isinstance(node.op, ast.And):
            return all(values)
        elif isinstance(node.op, ast.Or):
            return any(values)
        raise ValueError(f"Unsupported boolean operator: {type(node.op)}")

    def visit_Call(self, node):
        args = [self.visit(arg) for arg in node.args]
        kwargs = {kw.arg: self.visit(kw.value) for kw in node.keywords}

        if isinstance(node.func, ast.Name):
            func_name = node.func.id
            if func_name not in self.allowed_functions:
                raise ValueError(f"Function {func_name} is not allowed")
            return self.allowed_functions[func_name](*args, **kwargs)

        elif isinstance(node.func, ast.Attribute):
            obj = self.visit(node.func.value)
            attr = node.func.attr
            if attr.startswith('_'):
                raise ValueError(f"Access to private method {attr} is not allowed")
            if hasattr(obj, attr):
                method = getattr(obj, attr)
                if callable(method):
                    return method(*args, **kwargs)
            raise ValueError(f"Attribute or method {attr} is not allowed or not callable")

        raise ValueError("Unsupported function call")

    def visit_Name(self, node):
        raise ValueError(f"Variable {node.id} is not allowed")

    def visit_Subscript(self, node):
        value = self.visit(node.value)
        slice_val = self.visit(node.slice)
        return value[slice_val]

    def visit_Attribute(self, node):
        value = self.visit(node.value)
        if node.attr.startswith('_'):
            raise ValueError(f"Access to private attribute {node.attr} is not allowed")
        return getattr(value, node.attr)

    def visit_IfExp(self, node):
        test = self.visit(node.test)
        if test:
            return self.visit(node.body)
        else:
            return self.visit(node.orelse)

    def visit_List(self, node):
        return [self.visit(elt) for elt in node.elts]

    def visit_Tuple(self, node):
        return tuple(self.visit(elt) for elt in node.elts)

    def visit_Dict(self, node):
        return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}

    def visit_Set(self, node):
        return {self.visit(elt) for elt in node.elts}

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

def safe_eval(expr):
    tree = ast.parse(expr, mode='eval')
    evaluator = SafeEvaluator()
    return evaluator.visit(tree.body)
