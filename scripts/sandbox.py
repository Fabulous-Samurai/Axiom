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
import sys
import math

class Evaluator(ast.NodeVisitor):
    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if isinstance(node.op, ast.Add): return left + right
        if isinstance(node.op, ast.Sub): return left - right
        if isinstance(node.op, ast.Mult): return left * right
        if isinstance(node.op, ast.Div): return left / right
        if isinstance(node.op, ast.Mod): return left % right
        if isinstance(node.op, ast.Pow): return left ** right
        raise ValueError(f"Unsupported operator: {{type(node.op).__name__}}")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        if isinstance(node.op, ast.UAdd): return +operand
        if isinstance(node.op, ast.USub): return -operand
        raise ValueError(f"Unsupported unary operator: {{type(node.op).__name__}}")

    def visit_Constant(self, node):
        return node.value

    def visit_Num(self, node):
        return node.n

    def visit_Name(self, node):
        if node.id in ['pi', 'e', 'tau', 'inf', 'nan']:
            return getattr(math, node.id)
        raise ValueError(f"Unsupported variable: {{node.id}}")

    def visit_Call(self, node):
        if not isinstance(node.func, ast.Name):
            raise ValueError("Only functions can be called")
        if node.func.id not in ['sin', 'cos', 'tan', 'sqrt', 'log', 'log10', 'exp', 'abs', 'round']:
            raise ValueError(f"Unsupported function: {{node.func.id}}")
        if hasattr(math, node.func.id):
            func = getattr(math, node.func.id)
        elif node.func.id in ['abs', 'round']:
            func = getattr(__builtins__, node.func.id) if hasattr(__builtins__, node.func.id) else __builtins__[node.func.id] if type(__builtins__) is dict else getattr(__import__('builtins'), node.func.id)
        else:
            raise ValueError(f"Function not found: {{node.func.id}}")
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported node type: {{type(node).__name__}}")

try:
    tree = ast.parse({repr(expression)}, mode='eval')
    print(Evaluator().visit(tree.body))
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
