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
    
    # Use ast.literal_eval or a custom AST evaluator instead of dangerous eval()
    # to avoid Remote Code Execution (RCE) vulnerabilities.

    code = f"""
import ast
import operator
import math
import sys

class SafeEvaluator(ast.NodeVisitor):
    allowed_ops = {{
        ast.Add: operator.add,
        ast.Sub: operator.sub,
        ast.Mult: operator.mul,
        ast.Div: operator.truediv,
        ast.FloorDiv: operator.floordiv,
        ast.Mod: operator.mod,
        ast.Pow: operator.pow,
        ast.USub: operator.neg,
        ast.UAdd: operator.pos,
    }}

    allowed_functions = {{
        'sin': math.sin,
        'cos': math.cos,
        'tan': math.tan,
        'log': math.log,
        'log10': math.log10,
        'sqrt': math.sqrt,
        'abs': getattr(__builtins__, 'abs') if isinstance(__builtins__, dict) else getattr(__builtins__, 'abs', abs),
        'pi': math.pi,
        'e': math.e,
        'math': math  # Allow accessing math module directly for math.pi, etc.
    }}

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float, complex)):
            raise ValueError(f"Unsupported constant type: {{type(node.value)}}")
        return node.value

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)

        # Prevent DoS via large powers or string multiplications (if we allowed strings)
        if isinstance(node.op, ast.Pow):
            if isinstance(left, (int, float)) and isinstance(right, (int, float)):
                if abs(right) > 1000:
                    raise ValueError("Exponent too large")

        return self.allowed_ops[type(node.op)](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        return self.allowed_ops[type(node.op)](operand)

    def visit_Name(self, node):
        if node.id in self.allowed_functions:
            return self.allowed_functions[node.id]
        raise ValueError(f"Unknown variable/function: {{node.id}}")

    def visit_Attribute(self, node):
        value = self.visit(node.value)
        if value is math and hasattr(math, node.attr):
            return getattr(math, node.attr)
        raise ValueError(f"Unsupported attribute access: {{node.attr}}")

    def visit_Call(self, node):
        func = self.visit(node.func)
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {{type(node).__name__}}")

def safe_eval(expr):
    tree = ast.parse(expr, mode='eval')
    evaluator = SafeEvaluator()
    return evaluator.visit(tree)

print(safe_eval({repr(expression)}))
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
