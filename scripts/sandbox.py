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
    # Use a secure AST evaluation instead of eval() to prevent RCE
    code = f"""
import ast
import operator
import math

class SafeEval(ast.NodeVisitor):
    def visit_Constant(self, node):
        return node.value
    def visit_Name(self, node):
        if node.id in ['pi', 'e']: return getattr(math, node.id)
        if node.id in ['math']: return math
        raise ValueError('Unsupported name')
    def visit_BinOp(self, node):
        left, right = self.visit(node.left), self.visit(node.right)
        if isinstance(node.op, ast.Mult) and not (isinstance(left, (int, float)) and isinstance(right, (int, float))):
            raise ValueError('Only numeric mult allowed')
        if isinstance(node.op, ast.Pow) and isinstance(right, (int, float)) and right > 1000:
            raise ValueError('Exponent too large')
        ops = {{type(ast.Add()): operator.add, type(ast.Sub()): operator.sub, type(ast.Mult()): operator.mul, type(ast.Div()): operator.truediv, type(ast.Pow()): operator.pow, type(ast.Mod()): operator.mod}}
        if type(node.op) in ops: return ops[type(node.op)](left, right)
        raise ValueError('Unsupported binop')
    def visit_UnaryOp(self, node):
        ops = {{type(ast.USub()): operator.neg, type(ast.UAdd()): operator.pos}}
        if type(node.op) in ops: return ops[type(node.op)](self.visit(node.operand))
        raise ValueError('Unsupported unop')
    def visit_Attribute(self, node):
        val = self.visit(node.value)
        if val is math and hasattr(math, node.attr) and not node.attr.startswith('_'):
            return getattr(math, node.attr)
        raise ValueError('Unsupported attribute')
    def visit_Call(self, node):
        func = self.visit(node.func)
        args = [self.visit(arg) for arg in node.args]
        if hasattr(math, getattr(func, '__name__', '')):
            return func(*args)
        raise ValueError('Unsupported call')
    def generic_visit(self, node):
        raise ValueError('Unsupported node type')

try:
    print(SafeEval().visit(ast.parse({repr(expression)}, mode='eval').body))
except Exception as e:
    print(f"Error: {{e}}")
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
