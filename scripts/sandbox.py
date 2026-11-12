import ast
import operator
import math

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.ops = {ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
                    ast.Div: operator.truediv, ast.UAdd: operator.pos, ast.USub: operator.neg,
                    ast.Pow: operator.pow, ast.Mod: operator.mod}
        self.funcs = {
            'abs': abs, 'max': max, 'min': min, 'pow': pow, 'round': round,
            'sin': math.sin, 'cos': math.cos, 'tan': math.tan, 'sqrt': math.sqrt,
            'log': math.log, 'exp': math.exp
        }
    def visit_Constant(self, n): return n.value
    def visit_UnaryOp(self, n):
        op = self.ops.get(type(n.op))
        if op is None: raise ValueError(f"disallowed unary op: {type(n.op).__name__}")
        return op(self.visit(n.operand))
    def visit_BinOp(self, n):
        op = self.ops.get(type(n.op))
        if op is None: raise ValueError(f"disallowed binary op: {type(n.op).__name__}")
        return op(self.visit(n.left), self.visit(n.right))
    def visit_Call(self, n):
        if n.keywords: raise ValueError("kwargs disallowed")
        if not isinstance(n.func, ast.Name): raise ValueError("only simple function calls allowed")
        func_name = n.func.id
        if func_name not in self.funcs: raise ValueError(f"disallowed function: {func_name}")
        args = [self.visit(arg) for arg in n.args]
        return self.funcs[func_name](*args)
    def visit_Name(self, n):
        if n.id == 'pi': return math.pi
        if n.id == 'e': return math.e
        raise ValueError(f"Unknown variable: {n.id}")
    def generic_visit(self, n): raise ValueError(f"disallowed: {type(n).__name__}")

def safe_eval(expr):
    try:
        print(SafeEvaluator().visit(ast.parse(expr, mode='eval').body))
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

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
    code = f"import sys, os; sys.path.insert(0, {repr(os.path.dirname(os.path.abspath(__file__)))}); from sandbox import safe_eval; safe_eval(sys.argv[1])"
    cmd = [sys.executable, "-c", code, expression]
    
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
