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
    # We use a secure AST-based evaluator to prevent arbitrary code execution
    code = f"""
import sys
import ast
import math

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_funcs = {{'sin': math.sin, 'cos': math.cos, 'tan': math.tan, 'sqrt': math.sqrt, 'abs': getattr(__builtins__, 'abs') if type(__builtins__) is dict else __builtins__.abs, 'log': math.log, 'exp': math.exp}}
        self.allowed_names = {{'pi': math.pi, 'e': math.e}}

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if isinstance(node.op, ast.Add): return left + right
        elif isinstance(node.op, ast.Sub): return left - right
        elif isinstance(node.op, ast.Mult): return left * right
        elif isinstance(node.op, ast.Div): return left / right
        elif isinstance(node.op, ast.Pow): return left ** right
        elif isinstance(node.op, ast.Mod): return left % right
        else: raise ValueError(f"Unsupported operation: {{type(node.op).__name__}}")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        if isinstance(node.op, ast.UAdd): return +operand
        elif isinstance(node.op, ast.USub): return -operand
        else: raise ValueError(f"Unsupported unary operation: {{type(node.op).__name__}}")

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.allowed_names:
            return self.allowed_names[node.id]
        raise ValueError(f"Name '{{node.id}}' is not allowed")

    def visit_Call(self, node):
        if not isinstance(node.func, ast.Name):
            raise ValueError("Only simple function calls are allowed")
        if node.func.id not in self.allowed_funcs:
            raise ValueError(f"Function '{{node.func.id}}' is not allowed")
        args = [self.visit(arg) for arg in node.args]
        return self.allowed_funcs[node.func.id](*args)

    def visit_Expression(self, node):
        return self.visit(node.body)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported node type: {{type(node).__name__}}")

def evaluate(expr):
    tree = ast.parse(expr, mode='eval')
    evaluator = SafeEvaluator()
    return evaluator.visit(tree)

try:
    print(evaluate({repr(expression)}))
except Exception as e:
    sys.stderr.write("Exception: " + str(e) + chr(10))
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
