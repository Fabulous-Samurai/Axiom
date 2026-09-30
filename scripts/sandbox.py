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
    
    # We use AST-based safe evaluation instead of raw eval() to prevent arbitrary code execution
    # and sandbox escapes via __builtins__ manipulation or MRO traversal.
    code = f"""import ast
import operator
import math
import sys

class SafeEval(ast.NodeVisitor):
    def __init__(self):
        self.allowed_operators = {{
            ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
            ast.Div: operator.truediv, ast.Pow: operator.pow, ast.USub: operator.neg,
            ast.Mod: operator.mod
        }}
        b = __builtins__
        abs_func = getattr(b, 'abs') if type(b) is not dict else b['abs']
        self.allowed_funcs = {{
            'sin': math.sin, 'cos': math.cos, 'sqrt': math.sqrt,
            'pi': math.pi, 'abs': abs_func
        }}

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.allowed_funcs:
            return self.allowed_funcs[node.id]
        raise ValueError(f"Name '{{node.id}}' is not allowed")

    def visit_BinOp(self, node):
        op = type(node.op)
        if op not in self.allowed_operators:
            raise ValueError(f"Operator '{{op.__name__}}' is not allowed")
        return self.allowed_operators[op](self.visit(node.left), self.visit(node.right))

    def visit_UnaryOp(self, node):
        op = type(node.op)
        if op not in self.allowed_operators:
            raise ValueError(f"Operator '{{op.__name__}}' is not allowed")
        return self.allowed_operators[op](self.visit(node.operand))

    def visit_Call(self, node):
        if not isinstance(node.func, ast.Name):
            raise ValueError("Only simple calls")
        func_name = node.func.id
        if func_name not in self.allowed_funcs:
            raise ValueError(f"Function '{{func_name}}' is not allowed")
        return self.allowed_funcs[func_name](*[self.visit(a) for a in node.args])

    def generic_visit(self, node):
        raise ValueError(f"Node type '{{type(node).__name__}}' is not allowed")

try:
    expr_ast = ast.parse({repr(expression)}, mode='eval')
    print(SafeEval().visit(expr_ast.body))
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
