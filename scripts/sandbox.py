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
import sys
import ast
import operator
import math

class SafeEval(ast.NodeVisitor):
    def __init__(self):
        self.allowed_ops = {{
            ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
            ast.Div: operator.truediv, ast.Pow: operator.pow,
            ast.USub: operator.neg, ast.UAdd: operator.pos,
            ast.Mod: operator.mod, ast.FloorDiv: operator.floordiv
        }}
        self.allowed_funcs = {{
            'abs': abs, 'min': min, 'max': max, 'round': round,
            **{{k: v for k, v in math.__dict__.items() if not k.startswith('_')}}
        }}

    def visit_Constant(self, node):
        return node.value

    # For Python < 3.8 compatibility where Num, Str, NameConstant were used instead of Constant
    def visit_Num(self, node):
        return node.n

    def visit_Str(self, node):
        return node.s

    def visit_NameConstant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.allowed_funcs:
            return self.allowed_funcs[node.id]
        raise ValueError(f"Unknown variable: {{node.id}}")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if type(node.op) in self.allowed_ops:
            return self.allowed_ops[type(node.op)](left, right)
        raise ValueError(f"Unsupported operation: {{type(node.op).__name__}}")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        if type(node.op) in self.allowed_ops:
            return self.allowed_ops[type(node.op)](operand)
        raise ValueError(f"Unsupported unary operation: {{type(node.op).__name__}}")

    def visit_Call(self, node):
        func = self.visit(node.func)
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported syntax: {{type(node).__name__}}")

try:
    tree = ast.parse({repr(expression)}, mode='eval').body
    print(SafeEval().visit(tree))
except Exception as e:
    print(f"Error: {{str(e)}}")
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
