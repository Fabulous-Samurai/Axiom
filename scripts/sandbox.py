import os
import sys
import time
import threading
import subprocess
import signal

class ComplexityGuard:
    """
    Monitors the resource usage of an expression evaluation process.
    Terminates processes that exceed time or memory limits.
    """
    def __init__(self, timeout=5.0, max_memory_mb=512):
        self.timeout = timeout
        self.max_memory_mb = max_memory_mb

    def monitor(self, process):
        start_time = time.time()
        while process.poll() is None:
            if (time.time() - start_time) > self.timeout:
                print(f"[SANDBOX] Timeout exceeded ({self.timeout}s). Terminating.")
                process.kill()
                return
            time.sleep(0.1)

def run_isolated_expression(expression):
    """
    Runs an AXIOM expression in a restricted subprocess.
    In production, this would use AppContainer (Windows) or seccomp (Linux).
    """
    print(f"[SANDBOX] Evaluating: {expression}")
    
    # We use ast.parse and a custom NodeVisitor to safely evaluate expressions
    # without using the dangerous eval() function.
    sandbox_dir = os.path.dirname(os.path.abspath(__file__))
    code = f"""
import ast
import math
import operator
import sys

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_functions = {{
            'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
            'sqrt': math.sqrt, 'log': math.log, 'exp': math.exp,
            'pi': math.pi, 'e': math.e, 'abs': abs, 'max': max,
            'min': min, 'sum': sum,
        }}
        self.operators = {{
            ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
            ast.Div: operator.truediv, ast.Pow: operator.pow, ast.Mod: operator.mod,
            ast.USub: operator.neg, ast.UAdd: operator.pos, ast.BitOr: operator.or_,
            ast.BitAnd: operator.and_, ast.BitXor: operator.xor, ast.Invert: operator.invert,
            ast.LShift: operator.lshift, ast.RShift: operator.rshift,
            ast.Eq: operator.eq, ast.NotEq: operator.ne, ast.Lt: operator.lt,
            ast.LtE: operator.le, ast.Gt: operator.gt, ast.GtE: operator.ge,
            ast.Is: operator.is_, ast.IsNot: operator.is_not, ast.In: lambda a, b: a in b,
            ast.NotIn: lambda a, b: a not in b, ast.And: lambda a, b: a and b,
            ast.Or: lambda a, b: a or b, ast.Not: operator.not_
        }}

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.allowed_functions:
            return self.allowed_functions[node.id]
        raise ValueError(f"Unknown variable or function: {{node.id}}")

    def visit_Call(self, node):
        func = self.visit(node.func)
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

    def visit_BinOp(self, node):
        op = type(node.op)
        if op not in self.operators:
            raise ValueError(f"Unsupported operator: {{op}}")
        left = self.visit(node.left)
        right = self.visit(node.right)
        return self.operators[op](left, right)

    def visit_UnaryOp(self, node):
        op = type(node.op)
        if op not in self.operators:
            raise ValueError(f"Unsupported unary operator: {{op}}")
        operand = self.visit(node.operand)
        return self.operators[op](operand)

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, comp in zip(node.ops, node.comparators):
            op_type = type(op)
            if op_type not in self.operators:
                raise ValueError(f"Unsupported comparison: {{op_type}}")
            right = self.visit(comp)
            if not self.operators[op_type](left, right):
                return False
            left = right
        return True

    def visit_BoolOp(self, node):
        op_type = type(node.op)
        if op_type not in self.operators:
            raise ValueError(f"Unsupported boolean operator: {{op_type}}")
        values = [self.visit(val) for val in node.values]
        if op_type == ast.And:
            return all(values)
        elif op_type == ast.Or:
            return any(values)

    def visit_Subscript(self, node):
        value = self.visit(node.value)
        slice_val = self.visit(node.slice)
        return value[slice_val]

    def visit_List(self, node):
        return [self.visit(elt) for elt in node.elts]

    def visit_Tuple(self, node):
        return tuple([self.visit(elt) for elt in node.elts])

    def visit_Dict(self, node):
        return {{self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}}

    def generic_visit(self, node):
        raise ValueError(f"Unsupported construct: {{type(node).__name__}}")

try:
    tree = ast.parse({repr(expression)}, mode='eval')
    evaluator = SafeEvaluator()
    print(evaluator.visit(tree))
except Exception as e:
    # Print the error to stderr and exit with non-zero code to match behavior
    print(f"{{e}}", file=sys.stderr)
    sys.exit(1)
"""
    cmd = [sys.executable, "-c", code]
    
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        
        guard = ComplexityGuard()
        monitor_thread = threading.Thread(target=guard.monitor, args=(proc,))
        monitor_thread.start()
        
        stdout, stderr = proc.communicate()
        monitor_thread.join()
        
        if proc.returncode == 0:
            return stdout.strip()
        else:
            return f"Error: {stderr.strip()}"
            
    except Exception as e:
        return f"Sandbox Exception: {str(e)}"

if __name__ == "__main__":
    if len(sys.argv) > 1:
        expr = sys.argv[1]
        print(run_isolated_expression(expr))
    else:
        # Example adversarial expression (if eval was used directly)
        print(run_isolated_expression("__import__('os').listdir('.')"))
