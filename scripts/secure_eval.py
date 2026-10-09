import ast
import math
import operator
import sys

class SecureEvaluator(ast.NodeVisitor):
    ALLOWED_FUNCTIONS = {
        'sin': math.sin,
        'cos': math.cos,
        'tan': math.tan,
        'sqrt': math.sqrt,
        'abs': abs,
        'min': min,
        'max': max,
        'round': round,
    }

    ALLOWED_CONSTANTS = {
        'pi': math.pi,
        'e': math.e,
        'True': True,
        'False': False,
        'None': None
    }

    ALLOWED_OPERATORS = {
        ast.Add: operator.add,
        ast.Sub: operator.sub,
        ast.Mult: operator.mul,
        ast.Div: operator.truediv,
        ast.FloorDiv: operator.floordiv,
        ast.Mod: operator.mod,
        ast.Pow: operator.pow,
        ast.USub: operator.neg,
        ast.UAdd: operator.pos,
        ast.BitXor: operator.xor,
    }

    def evaluate(self, expr_string):
        tree = ast.parse(expr_string, mode='eval')
        return self.visit(tree.body)

    def visit_Constant(self, node):
        return node.value

    def visit_Num(self, node):
        return node.n

    def visit_Name(self, node):
        if node.id in self.ALLOWED_CONSTANTS:
            return self.ALLOWED_CONSTANTS[node.id]
        raise ValueError(f"Name '{node.id}' is not allowed")

    def visit_Call(self, node):
        if not isinstance(node.func, ast.Name):
            raise ValueError("Only simple function calls are allowed")
        func_name = node.func.id
        if func_name not in self.ALLOWED_FUNCTIONS:
            raise ValueError(f"Function '{func_name}' is not allowed")

        args = [self.visit(arg) for arg in node.args]
        return self.ALLOWED_FUNCTIONS[func_name](*args)

    def visit_BinOp(self, node):
        op_type = type(node.op)
        if op_type not in self.ALLOWED_OPERATORS:
            raise ValueError(f"Operator {op_type.__name__} is not allowed")

        left = self.visit(node.left)
        right = self.visit(node.right)

        if op_type == ast.Mult:
            if not (isinstance(left, (int, float)) and isinstance(right, (int, float))):
                raise ValueError("Multiplication is only allowed between numbers")
        elif op_type == ast.Pow:
            if not (isinstance(left, (int, float)) and isinstance(right, (int, float))):
                raise ValueError("Exponentiation is only allowed between numbers")

        return self.ALLOWED_OPERATORS[op_type](left, right)

    def visit_UnaryOp(self, node):
        op_type = type(node.op)
        if op_type not in self.ALLOWED_OPERATORS:
            raise ValueError(f"Operator {op_type.__name__} is not allowed")
        operand = self.visit(node.operand)
        return self.ALLOWED_OPERATORS[op_type](operand)

    def generic_visit(self, node):
        raise ValueError(f"AST node type {type(node).__name__} is not supported")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.exit(1)

    expr = sys.argv[1]
    try:
        evaluator = SecureEvaluator()
        print(evaluator.evaluate(expr))
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
