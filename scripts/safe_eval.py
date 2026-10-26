import os
import sys
import time
import threading
import subprocess
import ast
import operator

class SafeEvaluator(ast.NodeVisitor):
    allowed_ops = {
        ast.Add: operator.add,
        ast.Sub: operator.sub,
        ast.Mult: operator.mul,
        ast.Div: operator.truediv,
        ast.FloorDiv: operator.floordiv,
        ast.Mod: operator.mod,
        ast.Pow: operator.pow,
        ast.USub: operator.neg,
        ast.UAdd: operator.pos,
    }

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        raise ValueError(f"Variables are not allowed: {node.id}")

    def visit_UnaryOp(self, node):
        op = type(node.op)
        if op not in self.allowed_ops:
            raise ValueError(f"Unsupported unary operator: {op.__name__}")
        operand = self.visit(node.operand)
        return self.allowed_ops[op](operand)

    def visit_BinOp(self, node):
        op = type(node.op)
        if op not in self.allowed_ops:
            raise ValueError(f"Unsupported binary operator: {op.__name__}")

        left = self.visit(node.left)
        right = self.visit(node.right)

        if op == ast.Mult:
            if isinstance(left, str) or isinstance(right, str):
                raise ValueError("String multiplication is not allowed")
            if isinstance(left, list) or isinstance(right, list):
                raise ValueError("List multiplication is not allowed")

        return self.allowed_ops[op](left, right)

    def visit_Call(self, node):
        if node.keywords:
            raise ValueError("Keyword arguments are not allowed in function calls")
        func_name = getattr(node.func, 'id', None)
        if func_name not in {'abs', 'round', 'min', 'max'}:
            raise ValueError(f"Unsupported function: {func_name}")

        args = [self.visit(arg) for arg in node.args]
        return {"abs": abs, "round": round, "min": min, "max": max}[func_name](*args)

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_List(self, node):
        raise ValueError("List literals are not allowed")

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

def safe_eval(expr):
    tree = ast.parse(expr, mode='eval')
    evaluator = SafeEvaluator()
    return evaluator.visit(tree)

if __name__ == '__main__':
    if len(sys.argv) > 1:  # NOSONAR
        try:
            print(safe_eval(sys.argv[1]))
        except Exception as e:
            print(f"{type(e).__name__}: {str(e)}", file=sys.stderr)
            sys.exit(1)
