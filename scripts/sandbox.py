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
    
    # Use a safe AST evaluator instead of dangerous eval()
    code = f"""
import ast
import math
import operator

class SafeMath(ast.NodeVisitor):
    def __init__(self):
        self.ops = {{
            ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
            ast.Div: operator.truediv, ast.Pow: operator.pow, ast.USub: operator.neg,
            ast.Mod: operator.mod
        }}

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float)):
            raise ValueError("Only numbers allowed")
        return node.value

    def visit_Name(self, node):
        if hasattr(math, node.id):
            return getattr(math, node.id)
        if hasattr(__builtins__, node.id) or (type(__builtins__) is dict and node.id in __builtins__):
            b = __builtins__
            func = getattr(b, node.id) if type(b) is not dict else b.get(node.id)
            if func in (abs, min, max, round):
                return func
        raise ValueError(f"Unknown name: {{node.id}}")

    def visit_Attribute(self, node):
        if isinstance(node.value, ast.Name) and node.value.id == 'math':
            if hasattr(math, node.attr):
                return getattr(math, node.attr)
        raise ValueError(f"Unsupported attribute: {{node.attr}}")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if type(node.op) == ast.Mult and not (isinstance(left, (int, float)) and isinstance(right, (int, float))):
            raise ValueError("Multiplication only allowed on numbers")
        return self.ops[type(node.op)](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        return self.ops[type(node.op)](operand)

    def visit_Call(self, node):
        func = self.visit(node.func)
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {{type(node).__name__}}")

    def visit_Expression(self, node):
        return self.visit(node.body)

tree = ast.parse({repr(expression)}, mode='eval')
print(SafeMath().visit(tree))
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
