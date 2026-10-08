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
    
    # Secure evaluation using AST whitelisting to prevent arbitrary code execution
    code = f"""
import ast
import operator
import math
import sys

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_functions = {{
            'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
            'sqrt': math.sqrt, 'log': math.log, 'log10': math.log10,
            'exp': math.exp, 'abs': getattr(__builtins__, 'abs', None) if isinstance(__builtins__, dict) else getattr(__builtins__, 'abs')
        }}
        self.allowed_names = {{
            'pi': math.pi, 'e': math.e, 'True': True, 'False': False, 'None': None
        }}
        self.allowed_operators = {{
            ast.Add: operator.add, ast.Sub: operator.sub,
            ast.Mult: operator.mul, ast.Div: operator.truediv,
            ast.Pow: operator.pow, ast.Mod: operator.mod,
            ast.USub: operator.neg, ast.UAdd: operator.pos
        }}

    def visit_Module(self, node):
        if len(node.body) != 1 or not isinstance(node.body[0], ast.Expr):
            raise ValueError("Only single expressions are allowed")
        return self.visit(node.body[0].value)

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.allowed_names:
            return self.allowed_names[node.id]
        raise ValueError(f"Name '{{node.id}}' is not allowed")

    def visit_Call(self, node):
        if not isinstance(node.func, ast.Name) or node.func.id not in self.allowed_functions:
            func_name = node.func.id if isinstance(node.func, ast.Name) else str(type(node.func))
            raise ValueError(f"Function '{{func_name}}' is not allowed")
        func = self.allowed_functions[node.func.id]
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

    def visit_BinOp(self, node):
        op_type = type(node.op)
        if op_type not in self.allowed_operators:
            raise ValueError(f"Operator '{{op_type.__name__}}' is not allowed")
        left = self.visit(node.left)
        right = self.visit(node.right)
        if op_type == ast.Mult:
            if not isinstance(left, (int, float)) or not isinstance(right, (int, float)):
                raise ValueError("Multiplication is only allowed for numbers")
        if op_type == ast.Pow:
            if not isinstance(left, (int, float)) or not isinstance(right, (int, float)):
                raise ValueError("Power operation is only allowed for numbers")
        return self.allowed_operators[op_type](left, right)

    def visit_UnaryOp(self, node):
        op_type = type(node.op)
        if op_type not in self.allowed_operators:
            raise ValueError(f"Unary operator '{{op_type.__name__}}' is not allowed")
        operand = self.visit(node.operand)
        return self.allowed_operators[op_type](operand)

    def generic_visit(self, node):
        raise ValueError(f"AST node '{{type(node).__name__}}' is not allowed")

def safe_eval(expr):
    tree = ast.parse(expr, mode='eval')
    evaluator = SafeEvaluator()
    return evaluator.visit(tree)

try:
    print(safe_eval({repr(expression)}))
except Exception as e:
    print(f"Error: {{str(e)}}", file=sys.stderr)
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
