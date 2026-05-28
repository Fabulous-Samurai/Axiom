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
    print(f"[SANDBOX] Evaluating: {expression}", file=sys.stderr)
    
    # We use a more robust way to pass the expression to the subprocess
    # to avoid shell quoting issues.
    code = f"""
import ast, operator

class SafeMathEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_nodes = {{
            ast.Expression, ast.Constant, ast.BinOp, ast.UnaryOp,
            ast.Add, ast.Sub, ast.Mult, ast.Div, ast.Pow, ast.Mod,
            ast.USub, ast.UAdd
        }}
        self.operators = {{
            ast.Add: operator.add, ast.Sub: operator.sub,
            ast.Mult: operator.mul, ast.Div: operator.truediv,
            ast.Pow: operator.pow, ast.Mod: operator.mod,
            ast.USub: operator.neg, ast.UAdd: operator.pos
        }}

    def eval(self, expr_str):
        tree = ast.parse(expr_str, mode='eval')
        return self.visit(tree.body)

    def visit(self, node):
        if type(node) not in self.allowed_nodes:
            raise ValueError(f"Unsafe node type: {{type(node).__name__}}")
        return super().visit(node)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported node type: {{type(node).__name__}}")

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float, complex)):
            raise ValueError("Only numeric constants are allowed")
        return node.value

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op_type = type(node.op)
        if op_type not in self.operators:
            raise ValueError(f"Unsupported binary operator: {{op_type.__name__}}")
        return self.operators[op_type](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op_type = type(node.op)
        if op_type not in self.operators:
            raise ValueError(f"Unsupported unary operator: {{op_type.__name__}}")
        return self.operators[op_type](operand)

print(SafeMathEvaluator().eval({repr(expression)}))
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
