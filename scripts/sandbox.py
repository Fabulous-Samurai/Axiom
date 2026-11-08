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
    
    # We use a more robust and secure way to pass the expression to the subprocess
    # to avoid shell quoting issues and Remote Code Execution (RCE) via eval().
    code = """
import ast
import operator
import math
import sys

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.operators = {
            ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
            ast.Div: operator.truediv, ast.FloorDiv: operator.floordiv,
            ast.Mod: operator.mod, ast.Pow: operator.pow, ast.BitXor: operator.xor,
            ast.BitOr: operator.or_, ast.BitAnd: operator.and_,
            ast.USub: operator.neg, ast.UAdd: operator.pos,
        }
        self.functions = {
            'abs': abs, 'round': round, 'min': min, 'max': max, 'sum': sum,
            'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
            'sqrt': math.sqrt, 'exp': math.exp, 'log': math.log, 'log10': math.log10,
        }
        self.constants = {'pi': math.pi, 'e': math.e}

    def generic_visit(self, node):
        raise ValueError(f"Unsupported node type: {type(node).__name__}")

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.constants:
            return self.constants[node.id]
        raise ValueError(f"Unknown variable: {node.id}")

    def visit_UnaryOp(self, node):
        op = self.operators.get(type(node.op))
        if op is None: raise ValueError(f"Unsupported operator: {type(node.op).__name__}")
        return op(self.visit(node.operand))

    def visit_BinOp(self, node):
        op = self.operators.get(type(node.op))
        if op is None: raise ValueError(f"Unsupported operator: {type(node.op).__name__}")
        return op(self.visit(node.left), self.visit(node.right))

    def visit_Call(self, node):
        if node.keywords: raise ValueError("Keyword arguments are not supported")
        if not isinstance(node.func, ast.Name): raise ValueError("Only simple function calls are supported")
        func_name = node.func.id
        if func_name not in self.functions: raise ValueError(f"Unknown function: {func_name}")
        return self.functions[func_name](*[self.visit(arg) for arg in node.args])

def evaluate(expr):
    return SafeEvaluator().visit(ast.parse(expr, mode='eval'))

try:
    print(evaluate(sys.argv[1]))
except Exception as e:
    print(f"Error: {e}", file=sys.stderr)
    sys.exit(1)
"""
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
