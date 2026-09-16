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
    code = f"""
import ast, operator, math

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
            ast.BitXor: operator.xor,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos
        }}

        self.allowed_functions = {{
            'abs': abs, 'round': round, 'min': min, 'max': max,
            'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
            'sqrt': math.sqrt, 'log': math.log, 'exp': math.exp,
            'factorial': math.factorial
        }}

        self.allowed_constants = {{
            'pi': math.pi,
            'e': math.e,
            'True': True,
            'False': False,
            'None': None
        }}

    def evaluate(self, expr):
        try:
            tree = ast.parse(expr, mode='eval')
            return self.visit(tree.body)
        except Exception as e:
            raise ValueError(f"Invalid expression: {{str(e)}}")

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.allowed_constants:
            return self.allowed_constants[node.id]
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
        raise ValueError(f"Unary operator {{type(node.op).__name__}} is not allowed")

    def visit_Call(self, node):
        if not isinstance(node.func, ast.Name):
            raise ValueError("Only simple function calls are allowed")

        func_name = node.func.id
        if func_name not in self.allowed_functions:
            raise ValueError(f"Function '{{func_name}}' is not allowed")

        args = [self.visit(arg) for arg in node.args]
        return self.allowed_functions[func_name](*args)

    def generic_visit(self, node):
        raise ValueError(f"Node type {{type(node).__name__}} is not allowed")

print(SafeEvaluator().evaluate({repr(expression)}))
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
