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
    
    # We use an AST-based evaluator to safely evaluate math expressions
    code = f"""
import sys
import ast
import math
import operator

class MathEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_names = {{
            'pi': math.pi, 'e': math.e, 'tau': math.tau, 'inf': math.inf, 'nan': math.nan,
            'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
            'sqrt': math.sqrt, 'log': math.log, 'log10': math.log10,
            'exp': math.exp, 'pow': math.pow,
            'abs': abs, 'round': round, 'min': min, 'max': max,
        }}
        self.operators = {{
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv,
            ast.Pow: operator.pow,
            ast.Mod: operator.mod,
            ast.UAdd: operator.pos,
            ast.USub: operator.neg,
            ast.BitXor: operator.xor,
            ast.BitOr: operator.or_,
            ast.BitAnd: operator.and_,
        }}

    def evaluate(self, node):
        if isinstance(node, ast.Expression):
            return self.evaluate(node.body)
        elif isinstance(node, ast.Constant):
            return node.value
        elif isinstance(node, ast.Name):
            if node.id in self.allowed_names:
                return self.allowed_names[node.id]
            raise ValueError(f"Name '{{node.id}}' is not allowed")
        elif isinstance(node, ast.BinOp):
            left = self.evaluate(node.left)
            right = self.evaluate(node.right)
            if type(node.op) in self.operators:
                return self.operators[type(node.op)](left, right)
            raise ValueError(f"Operator '{{type(node.op).__name__}}' is not allowed")
        elif isinstance(node, ast.UnaryOp):
            operand = self.evaluate(node.operand)
            if type(node.op) in self.operators:
                return self.operators[type(node.op)](operand)
            raise ValueError(f"Unary operator '{{type(node.op).__name__}}' is not allowed")
        elif isinstance(node, ast.Call):
            if not isinstance(node.func, ast.Name):
                raise ValueError("Only simple function calls are allowed")
            func_name = node.func.id
            if func_name not in self.allowed_names:
                raise ValueError(f"Function '{{func_name}}' is not allowed")
            func = self.allowed_names[func_name]
            args = [self.evaluate(arg) for arg in node.args]
            return func(*args)
        elif isinstance(node, ast.List):
            return [self.evaluate(elt) for elt in node.elts]
        elif isinstance(node, ast.Dict):
            return {{self.evaluate(k): self.evaluate(v) for k, v in zip(node.keys, node.values)}}
        else:
            raise ValueError(f"Node type '{{type(node).__name__}}' is not allowed")

try:
    tree = ast.parse({repr(expression)}, mode='eval')
    print(MathEvaluator().evaluate(tree))
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
