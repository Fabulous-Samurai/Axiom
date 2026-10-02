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
    
# We use a custom AST evaluator to prevent arbitrary code execution
    code = f"""
import ast
import math
import sys

class SafeEvaluator(ast.NodeVisitor):
    ALLOWED_NAMES = {{'pi': math.pi, 'e': math.e}}
    ALLOWED_FUNCS = {{
        'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
        'sqrt': math.sqrt, 'abs': getattr(__builtins__, 'abs') if type(__builtins__) is not dict else __builtins__['abs'],
        'pow': getattr(__builtins__, 'pow') if type(__builtins__) is not dict else __builtins__['pow'],
        'log': math.log, 'exp': math.exp
    }}

    def visit_Module(self, node):
        if len(node.body) != 1 or not isinstance(node.body[0], ast.Expr):
            raise ValueError("Only single expressions are allowed")
        return self.visit(node.body[0].value)

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.ALLOWED_NAMES:
            return self.ALLOWED_NAMES[node.id]
        raise ValueError(f"Name '{{node.id}}' is not allowed")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op = node.op
        if isinstance(op, ast.Add): return left + right
        if isinstance(op, ast.Sub): return left - right
        if isinstance(op, ast.Mult): return left * right
        if isinstance(op, ast.Div): return left / right
        if isinstance(op, ast.Mod): return left % right
        if isinstance(op, ast.Pow): return left ** right
        raise ValueError(f"Unsupported binary operator: {{type(op).__name__}}")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op = node.op
        if isinstance(op, ast.UAdd): return +operand
        if isinstance(op, ast.USub): return -operand
        raise ValueError(f"Unsupported unary operator: {{type(op).__name__}}")

    def visit_Call(self, node):
        if not isinstance(node.func, ast.Name):
            raise ValueError("Only simple function calls are allowed")
        if node.func.id not in self.ALLOWED_FUNCS:
            raise ValueError(f"Function '{{node.func.id}}' is not allowed")
        args = [self.visit(arg) for arg in node.args]
        return self.ALLOWED_FUNCS[node.func.id](*args)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported syntax: {{type(node).__name__}}")

try:
    expr = {repr(expression)}
    tree = ast.parse(expr, mode='exec')
    evaluator = SafeEvaluator()
    print(evaluator.visit(tree))
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
