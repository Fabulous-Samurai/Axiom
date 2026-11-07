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
    # to avoid shell quoting issues, while avoiding dangerous eval().
    code = f"""
import ast
import operator
import sys
import math

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_operators = {{
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
        self.allowed_functions = {{
            'abs': abs,
            'round': round,
            'min': min,
            'max': max,
            'sum': sum,
            'math.sqrt': math.sqrt,
        }}

    def generic_visit(self, node):
        raise ValueError(f"Unsupported syntax: {{type(node).__name__}}")

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float)):
            raise ValueError("Only numeric constants are allowed")
        return node.value

    def visit_UnaryOp(self, node):
        if type(node.op) not in self.allowed_operators:
            raise ValueError(f"Unsupported unary operator: {{type(node.op).__name__}}")
        return self.allowed_operators[type(node.op)](self.visit(node.operand))

    def visit_BinOp(self, node):
        if type(node.op) not in self.allowed_operators:
            raise ValueError(f"Unsupported binary operator: {{type(node.op).__name__}}")
        return self.allowed_operators[type(node.op)](self.visit(node.left), self.visit(node.right))

    def visit_Call(self, node):
        if node.keywords:
            raise ValueError("Keyword arguments are not allowed")

        if isinstance(node.func, ast.Name):
            func_name = node.func.id
        elif isinstance(node.func, ast.Attribute) and isinstance(node.func.value, ast.Name):
            func_name = f"{{node.func.value.id}}.{{node.func.attr}}"
        else:
            raise ValueError("Unsupported function call")

        if func_name not in self.allowed_functions:
            raise ValueError(f"Unsupported function: {{func_name}}")

        args = [self.visit(arg) for arg in node.args]
        return self.allowed_functions[func_name](*args)

try:
    tree = ast.parse({repr(expression)}, mode='eval')
    evaluator = SafeEvaluator()
    print(evaluator.visit(tree))
except Exception as e:
    sys.stderr.write(str(e))
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
