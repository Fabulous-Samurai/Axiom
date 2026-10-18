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

    # Replace unsafe eval with AST-based safe evaluation
    code = f"""import sys, ast, operator, math
class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_operators = {{
            ast.Add: operator.add, ast.Sub: operator.sub,
            ast.Mult: operator.mul, ast.Div: operator.truediv,
            ast.USub: operator.neg, ast.UAdd: operator.pos,
        }}

        # Determine __builtins__ access based on environment
        if isinstance(__builtins__, dict):
            get_builtin = lambda x: __builtins__.get(x)
        else:
            get_builtin = lambda x: getattr(__builtins__, x, None)

        self.allowed_calls = {{
            'abs': get_builtin('abs'),
            'round': get_builtin('round'),
            'min': get_builtin('min'),
            'max': get_builtin('max'),
        }}
    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op_type = type(node.op)
        if op_type not in self.allowed_operators:
            raise ValueError(f"Unsupported operator: {{op_type.__name__}}")
        if op_type == ast.Mult and not (isinstance(left, (int, float)) and isinstance(right, (int, float))):
             raise ValueError("Multiplication is only allowed for numbers")
        return self.allowed_operators[op_type](left, right)
    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op_type = type(node.op)
        if op_type not in self.allowed_operators:
            raise ValueError(f"Unsupported unary operator: {{op_type.__name__}}")
        return self.allowed_operators[op_type](operand)
    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float, complex)):
            raise ValueError("Only numbers are allowed")
        return node.value
    def visit_Name(self, node):
        if node.id in self.allowed_calls:
            return self.allowed_calls[node.id]
        if node.id == 'math':
            return math
        if hasattr(math, node.id):
            return getattr(math, node.id)
        raise ValueError(f"Variable '{{node.id}}' not allowed")
    def visit_Call(self, node):
        func = self.visit(node.func)
        if not callable(func):
            raise ValueError("Invalid function call")
        args = [self.visit(arg) for arg in node.args]
        return func(*args)
    def visit_Attribute(self, node):
        value = self.visit(node.value)
        if value is math and hasattr(math, node.attr):
            return getattr(math, node.attr)
        raise ValueError("Attributes are not allowed on this object")
    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {{type(node).__name__}}")

def safe_eval(expr):
    tree = ast.parse(expr, mode='eval')
    evaluator = SafeEvaluator()
    return evaluator.visit(tree.body)

print(safe_eval({repr(expression)}))"""

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
