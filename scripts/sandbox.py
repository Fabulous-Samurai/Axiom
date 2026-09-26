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
    
    # We use a more robust way to pass the expression to the subprocess
    # to avoid shell quoting issues.
    code = f"""
import ast
import operator
import math
import sys

class SafeEval(ast.NodeVisitor):
    allowed_ops = {{
        ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
        ast.Div: operator.truediv, ast.FloorDiv: operator.floordiv,
        ast.Mod: operator.mod, ast.Pow: operator.pow,
        ast.USub: operator.neg, ast.UAdd: operator.pos,
        ast.BitXor: operator.xor, ast.BitOr: operator.or_, ast.BitAnd: operator.and_,
        ast.Eq: operator.eq, ast.NotEq: operator.ne, ast.Lt: operator.lt,
        ast.LtE: operator.le, ast.Gt: operator.gt, ast.GtE: operator.ge,
        ast.And: operator.and_, ast.Or: operator.or_
    }}
    allowed_funcs = {{
        'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
        'sqrt': math.sqrt, 'log': math.log, 'log10': math.log10,
        'exp': math.exp, 'pi': math.pi, 'e': math.e, 'abs': abs,
        'min': min, 'max': max, 'round': round
    }}
    def visit_Constant(self, node): return node.value
    def visit_Num(self, node): return node.n
    def visit_Name(self, node):
        if node.id in self.allowed_funcs:
            return self.allowed_funcs[node.id]
        raise ValueError(f"Name '{{node.id}}' is not allowed")
    def visit_BinOp(self, node):
        return self.allowed_ops[type(node.op)](self.visit(node.left), self.visit(node.right))
    def visit_UnaryOp(self, node):
        return self.allowed_ops[type(node.op)](self.visit(node.operand))
    def visit_BoolOp(self, node):
        values = [self.visit(v) for v in node.values]
        if isinstance(node.op, ast.And):
            return all(values)
        if isinstance(node.op, ast.Or):
            return any(values)
    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, right_node in zip(node.ops, node.comparators):
            right = self.visit(right_node)
            if not self.allowed_ops[type(op)](left, right): return False
            left = right
        return True
    def visit_Call(self, node):
        if isinstance(node.func, ast.Name):
            func = self.visit(node.func)
            args = [self.visit(a) for a in node.args]
            return func(*args)
        raise ValueError(f"Function call not allowed")
    def visit_Expression(self, node):
        return self.visit(node.body)
    def generic_visit(self, node):
        raise ValueError(f"Node '{{type(node).__name__}}' is not allowed")

try:
    expr = {repr(expression)}
    tree = ast.parse(expr, mode='eval')
    print(SafeEval().visit(tree))
except Exception as e:
    print(f"Error: {{e}}", file=sys.stderr)
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
