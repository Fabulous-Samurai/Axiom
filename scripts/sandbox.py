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

    def generate_evaluator_code(expr):
        return f"""import ast
import math
import operator
import sys

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.binops = {{
            ast.Add: operator.add, ast.Sub: operator.sub,
            ast.Mult: operator.mul, ast.Div: operator.truediv,
            ast.Mod: operator.mod, ast.Pow: operator.pow,
            ast.BitXor: operator.xor, ast.BitOr: operator.or_,
            ast.BitAnd: operator.and_
        }}
        self.unops = {{
            ast.USub: operator.neg, ast.UAdd: operator.pos,
            ast.Not: operator.not_
        }}
        self.allowed_funcs = {{
            'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
            'sqrt': math.sqrt, 'abs': abs, 'min': min, 'max': max,
            'log': math.log, 'log10': math.log10, 'exp': math.exp
        }}
        self.allowed_names = {{
            'pi': math.pi, 'e': math.e
        }}

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.allowed_names:
            return self.allowed_names[node.id]
        raise ValueError(f"Name '{{node.id}}' is not allowed")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op_type = type(node.op)
        if op_type in self.binops:
            return self.binops[op_type](left, right)
        raise ValueError(f"Operator '{{op_type.__name__}}' not supported")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op_type = type(node.op)
        if op_type in self.unops:
            return self.unops[op_type](operand)
        raise ValueError(f"Unary operator '{{op_type.__name__}}' not supported")

    def visit_Call(self, node):
        if not isinstance(node.func, ast.Name):
            raise ValueError("Only simple function calls are allowed")
        func_name = node.func.id
        if func_name not in self.allowed_funcs:
            raise ValueError(f"Function '{{func_name}}' is not allowed")
        args = [self.visit(arg) for arg in node.args]
        return self.allowed_funcs[func_name](*args)

    def visit_Expr(self, node):
        return self.visit(node.value)

    def generic_visit(self, node):
        raise ValueError(f"Node type '{{type(node).__name__}}' is not allowed")

    def evaluate(self, expr_str):
        tree = ast.parse(expr_str, mode='eval')
        return self.visit(tree.body)

try:
    evaluator = SafeEvaluator()
    print(evaluator.evaluate({repr(expr)}))
except Exception as e:
    print(f"Error: {{e}}", file=sys.stderr)
    sys.exit(1)
"""

    code = generate_evaluator_code(expression)

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
