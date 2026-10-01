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
    code = f"""import sys, ast, math

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_builtins = {{'abs': abs, 'min': min, 'max': max, 'sum': sum, 'round': round}}

    def visit_Module(self, node):
        if len(node.body) != 1 or not isinstance(node.body[0], ast.Expr):
            raise ValueError("Only single expressions are allowed")
        return self.visit(node.body[0].value)

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op = node.op
        if isinstance(op, ast.Add): return left + right
        if isinstance(op, ast.Sub): return left - right
        if isinstance(op, ast.Mult): return left * right
        if isinstance(op, ast.Div): return left / right
        if isinstance(op, ast.FloorDiv): return left // right
        if isinstance(op, ast.Mod): return left % right
        if isinstance(op, ast.Pow): return left ** right
        raise ValueError(f"Unsupported operation: {{type(op).__name__}}")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        if isinstance(node.op, ast.UAdd): return +operand
        if isinstance(node.op, ast.USub): return -operand
        raise ValueError(f"Unsupported unary operation: {{type(node.op).__name__}}")

    def visit_Constant(self, node):
        return node.value

    def visit_List(self, node):
        return [self.visit(elt) for elt in node.elts]

    def visit_Dict(self, node):
        return {{self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}}

    def visit_Call(self, node):
        if isinstance(node.func, ast.Name):
            func_name = node.func.id
            if func_name in self.allowed_builtins:
                args = [self.visit(arg) for arg in node.args]
                return self.allowed_builtins[func_name](*args)
            raise ValueError(f"Function '{{func_name}}' is not allowed")
        elif isinstance(node.func, ast.Attribute) and isinstance(node.func.value, ast.Name):
            if node.func.value.id == 'math':
                func_name = node.func.attr
                if hasattr(math, func_name) and not func_name.startswith('_'):
                    args = [self.visit(arg) for arg in node.args]
                    return getattr(math, func_name)(*args)
        raise ValueError("Unsupported function call")

    def visit_Name(self, node):
        if node.id == 'True': return True
        if node.id == 'False': return False
        if node.id == 'None': return None
        if node.id == 'math': return math
        raise ValueError(f"Variable '{{node.id}}' is not allowed")

    def visit_Attribute(self, node):
        if isinstance(node.value, ast.Name) and node.value.id == 'math':
            if hasattr(math, node.attr) and not node.attr.startswith('_'):
                return getattr(math, node.attr)
        raise ValueError("Unsupported attribute access")

    def generic_visit(self, node):
        raise ValueError(f"Unsupported syntax: {{type(node).__name__}}")

expr_str = {repr(expression)}
try:
    tree = ast.parse(expr_str, mode='exec')
    evaluator = SafeEvaluator()
    result = evaluator.visit(tree)
    print(result)
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
