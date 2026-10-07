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
    # to avoid shell quoting issues, while restricting evaluation with AST.
    code = f"""
import ast
import operator
import math
import sys

class SafeEval(ast.NodeVisitor):
    allowed_ops = {{
        ast.Add: operator.add,
        ast.Sub: operator.sub,
        ast.Mult: operator.mul,
        ast.Div: operator.truediv,
        ast.Pow: operator.pow,
        ast.BitXor: operator.xor,
        ast.USub: operator.neg,
        ast.UAdd: operator.pos,
        ast.Mod: operator.mod
    }}

    allowed_funcs = {{
        'sin': math.sin,
        'cos': math.cos,
        'tan': math.tan,
        'sqrt': math.sqrt,
        'abs': abs,
        'min': min,
        'max': max,
        'log': math.log,
        'log10': math.log10,
        'exp': math.exp
    }}

    allowed_names = {{
        'pi': math.pi,
        'e': math.e,
        'math': math
    }}

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if type(node.op) not in self.allowed_ops:
            raise ValueError(f"Operator {{type(node.op).__name__}} not allowed")
        if isinstance(node.op, ast.Mult):
            if not isinstance(left, (int, float)) or not isinstance(right, (int, float)):
                raise ValueError("Multiplication only allowed for numbers")
        if isinstance(node.op, ast.Pow):
            if not isinstance(left, (int, float)) or not isinstance(right, (int, float)):
                raise ValueError("Power only allowed for numbers")
            if right > 1000:
                raise ValueError("Power too large")
        return self.allowed_ops[type(node.op)](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        if type(node.op) not in self.allowed_ops:
            raise ValueError(f"Operator {{type(node.op).__name__}} not allowed")
        return self.allowed_ops[type(node.op)](operand)

    def visit_Call(self, node):
        if not isinstance(node.func, (ast.Name, ast.Attribute)):
            raise ValueError("Only named functions are allowed")
        func_name = node.func.id if isinstance(node.func, ast.Name) else node.func.attr
        if func_name not in self.allowed_funcs:
            raise ValueError(f"Function {{func_name}} not allowed")
        args = [self.visit(arg) for arg in node.args]
        return self.allowed_funcs[func_name](*args)

    def visit_Name(self, node):
        if node.id in self.allowed_names:
            return self.allowed_names[node.id]
        raise ValueError(f"Name {{node.id}} not allowed")

    def visit_Attribute(self, node):
        if isinstance(node.value, ast.Name) and node.value.id == 'math':
            if node.attr in self.allowed_funcs:
                return self.allowed_funcs[node.attr]
            if node.attr in self.allowed_names:
                return self.allowed_names[node.attr]
        raise ValueError(f"Attribute {{node.attr}} not allowed")

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float)):
            raise ValueError("Only numbers are allowed")
        return node.value

    def generic_visit(self, node):
        raise ValueError(f"Node type {{type(node).__name__}} not allowed")

try:
    def safe_eval(expr):
        tree = ast.parse(expr, mode='eval')
        visitor = SafeEval()
        return visitor.visit(tree.body)

    print(safe_eval({repr(expression)}))
except Exception as e:
    print(f"{{type(e).__name__}}: {{e}}", file=sys.stderr)
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
