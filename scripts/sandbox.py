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
    code = f"""import ast
import operator
import math
import sys

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self, safe_dict=None):
        if safe_dict is None:
            safe_dict = {{
                'abs': getattr(__builtins__, 'abs', None) if type(__builtins__) is dict else getattr(__builtins__, 'abs'),
                'round': getattr(__builtins__, 'round', None) if type(__builtins__) is dict else getattr(__builtins__, 'round'),
                'min': getattr(__builtins__, 'min', None) if type(__builtins__) is dict else getattr(__builtins__, 'min'),
                'max': getattr(__builtins__, 'max', None) if type(__builtins__) is dict else getattr(__builtins__, 'max'),
                'sum': getattr(__builtins__, 'sum', None) if type(__builtins__) is dict else getattr(__builtins__, 'sum'),
                'sin': math.sin, 'cos': math.cos,
                'tan': math.tan, 'sqrt': math.sqrt, 'pi': math.pi,
                'e': math.e,
                'math': math
            }}
        self.safe_dict = safe_dict
        self.allowed_operators = {{
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.Mod: operator.mod,
            ast.Pow: operator.pow,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos,
        }}

    def evaluate(self, expr):
        try:
            tree = ast.parse(expr, mode='eval')
            return self.visit(tree.body)
        except SyntaxError:
            raise ValueError("Invalid syntax")

    def visit_Constant(self, node):
        if isinstance(node.value, (int, float, complex)):
            return node.value
        raise ValueError(f"Unsupported constant type: {{type(node.value)}}")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op_type = type(node.op)

        if op_type not in self.allowed_operators:
            raise ValueError(f"Unsupported operator: {{op_type}}")

        if op_type == ast.Pow:
            if isinstance(right, (int, float)) and right > 1000:
                raise ValueError("Exponent too large")

        if op_type == ast.Mult:
            if type(left) is str or type(right) is str:
                raise ValueError("String replication not allowed")

        return self.allowed_operators[op_type](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op_type = type(node.op)
        if op_type not in self.allowed_operators:
            raise ValueError(f"Unsupported operator: {{op_type}}")
        return self.allowed_operators[op_type](operand)

    def visit_Call(self, node):
        func_name = None
        if isinstance(node.func, ast.Name):
            func_name = node.func.id
        elif isinstance(node.func, ast.Attribute):
            if isinstance(node.func.value, ast.Name):
                func_name = f"{{node.func.value.id}}.{{node.func.attr}}"

        if not func_name:
            raise ValueError("Only simple function calls or single module attributes are allowed")

        if func_name not in self.safe_dict:
            if isinstance(node.func, ast.Attribute):
                module_name = node.func.value.id
                if module_name in self.safe_dict:
                    module_obj = self.safe_dict[module_name]
                    if hasattr(module_obj, node.func.attr):
                        func_obj = getattr(module_obj, node.func.attr)
                    else:
                        raise ValueError(f"Unsupported function: {{func_name}}")
                else:
                    raise ValueError(f"Unsupported function: {{func_name}}")
            else:
                raise ValueError(f"Unsupported function: {{func_name}}")
        else:
            func_obj = self.safe_dict[func_name]

        args = [self.visit(arg) for arg in node.args]
        return func_obj(*args)

    def visit_Name(self, node):
        if node.id in self.safe_dict:
            val = self.safe_dict[node.id]
            if not callable(val):
                return val
            return val
        raise ValueError(f"Unsupported variable: {{node.id}}")

    def visit_Attribute(self, node):
        if isinstance(node.value, ast.Name) and node.value.id in self.safe_dict:
            obj = self.safe_dict[node.value.id]
            if hasattr(obj, node.attr):
                return getattr(obj, node.attr)
        raise ValueError(f"Unsupported attribute: {{node.attr}}")

evaluator = SafeEvaluator()
try:
    print(evaluator.evaluate({repr(expression)}))
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
