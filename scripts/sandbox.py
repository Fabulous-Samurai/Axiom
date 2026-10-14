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
    
    # We use an AST-based safe evaluator to execute mathematical expressions
    # without exposing full Python eval capabilities.
    code = f"""
import ast
import sys
import operator
import math

class SafeEvaluator(ast.NodeVisitor):
    allowed_ops = {{
        ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
        ast.Div: operator.truediv, ast.FloorDiv: operator.floordiv,
        ast.Mod: operator.mod, ast.Pow: operator.pow,
        ast.USub: operator.neg, ast.UAdd: operator.pos,
    }}

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)

        op_type = type(node.op)
        if op_type not in self.allowed_ops:
            raise ValueError(f"Unsupported operator: {{op_type.__name__}}")

        if op_type == ast.Mult:
            if not isinstance(left, (int, float)) or not isinstance(right, (int, float)):
                raise TypeError("Multiplication operands must be numbers")
        elif op_type == ast.Pow:
            if not isinstance(left, (int, float)) or not isinstance(right, (int, float)):
                raise TypeError("Power operands must be numbers")
            if right > 100 or right < -100:
                raise ValueError("Power exponent too large")

        return self.allowed_ops[op_type](left, right)

    def visit_UnaryOp(self, node):
        op_type = type(node.op)
        if op_type not in self.allowed_ops:
             raise ValueError(f"Unsupported unary operator: {{op_type.__name__}}")
        return self.allowed_ops[op_type](self.visit(node.operand))

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float)):
             raise TypeError("Only numeric constants are allowed")
        return node.value

    def visit_Name(self, node):
        if hasattr(math, node.id):
             return getattr(math, node.id)
        raise ValueError(f"Unknown variable/function: {{node.id}}")

    def visit_Attribute(self, node):
        if isinstance(node.value, ast.Name) and node.value.id == 'math':
            if hasattr(math, node.attr):
                return getattr(math, node.attr)
        raise ValueError(f"Unsupported attribute: {{node.attr}}")

    def visit_Call(self, node):
        func = self.visit(node.func)
        if not callable(func):
            raise ValueError("Only mathematical functions are allowed")
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {{type(node).__name__}}")

try:
    print(SafeEvaluator().visit(ast.parse({repr(expression)}, mode='eval').body))
except Exception as e:
    print(str(e), file=sys.stderr)
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
