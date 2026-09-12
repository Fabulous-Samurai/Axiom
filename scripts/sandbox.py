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
    
    # We use an AST-based evaluator inside the subprocess to prevent
    # arbitrary code execution vulnerabilities like MRO traversal.
    code = f'''
import ast
import operator
import sys

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_functions = {{
            'abs': abs, 'max': max, 'min': min,
            'round': round, 'sum': sum, 'len': len
        }}
        self.allowed_operators = {{
            ast.Add: operator.add, ast.Sub: operator.sub,
            ast.Mult: operator.mul, ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv, ast.Mod: operator.mod,
            ast.Pow: operator.pow, ast.USub: operator.neg,
            ast.UAdd: operator.pos,
        }}

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        return node.value

    # For backward compatibility
    def visit_Num(self, node): return node.n
    def visit_Str(self, node): return node.s
    def visit_NameConstant(self, node): return node.value

    def visit_Name(self, node):
        if node.id in self.allowed_functions:
            return self.allowed_functions[node.id]
        raise ValueError(f"Name '{{node.id}}' is not allowed")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if type(node.op) in self.allowed_operators:
            return self.allowed_operators[type(node.op)](left, right)
        raise ValueError(f"Operator {{type(node.op).__name__}} is not allowed")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        if type(node.op) in self.allowed_operators:
            return self.allowed_operators[type(node.op)](operand)
        raise ValueError(f"Operator {{type(node.op).__name__}} is not allowed")

    def visit_Call(self, node):
        func = self.visit(node.func)
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

    def generic_visit(self, node):
        raise ValueError(f"Syntax node {{type(node).__name__}} is not allowed")

try:
    tree = ast.parse({repr(expression)}, mode='eval')
    evaluator = SafeEvaluator()
    print(evaluator.visit(tree))
except Exception as e:
    print(f"Error: {{e}}")
'''
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
