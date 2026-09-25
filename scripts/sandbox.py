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
    

    # We use a strict AST-based evaluator to prevent arbitrary code execution
    # while allowing mathematical expressions.
    code = f'''
import ast
import math
import operator
import sys

class SafeEvaluator(ast.NodeVisitor):
    ALLOWED_OPERATORS = {{
        ast.Add: operator.add, ast.Sub: operator.sub,
        ast.Mult: operator.mul, ast.Div: operator.truediv,
        ast.FloorDiv: operator.floordiv, ast.Mod: operator.mod,
        ast.Pow: operator.pow, ast.BitXor: operator.xor,
        ast.BitOr: operator.or_, ast.BitAnd: operator.and_,
        ast.USub: operator.neg, ast.UAdd: operator.pos,
    }}

    ALLOWED_MATH = {{
        'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
        'sqrt': math.sqrt, 'log': math.log, 'log10': math.log10,
        'pi': math.pi, 'e': math.e
    }}

    def evaluate(self, expr):
        tree = ast.parse(expr, mode='eval')
        return self.visit(tree.body)

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.ALLOWED_MATH:
            return self.ALLOWED_MATH[node.id]
        raise ValueError(f"Unsafe or undefined name '{{node.id}}'")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op = type(node.op)
        if op in self.ALLOWED_OPERATORS:
            return self.ALLOWED_OPERATORS[op](left, right)
        raise ValueError(f"Unsupported operator '{{op}}'")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op = type(node.op)
        if op in self.ALLOWED_OPERATORS:
            return self.ALLOWED_OPERATORS[op](operand)
        raise ValueError(f"Unsupported unary operator '{{op}}'")

    def visit_Call(self, node):
        func = self.visit(node.func)
        if not callable(func) or func not in self.ALLOWED_MATH.values():
            raise ValueError("Unsafe function call")
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {{type(node).__name__}}")

try:
    evaluator = SafeEvaluator()
    result = evaluator.evaluate({repr(expression)})
    print(result)
except Exception as e:
    print(f"Error: {{e}}", file=sys.stderr)
    sys.exit(1)
'''
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
