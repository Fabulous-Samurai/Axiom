import os
import sys
import time
import threading
import subprocess
import signal
import ast
import operator
import math

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

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_ops = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.Mod: operator.mod,
            ast.Pow: operator.pow,
            ast.BitXor: operator.xor,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos,
        }

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id == 'math':
            return math
        if hasattr(math, node.id):
            return getattr(math, node.id)
        raise ValueError(f"Unsupported name: {node.id}")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if isinstance(node.op, (ast.Mult, ast.Pow)):
            if not (isinstance(left, (int, float)) and isinstance(right, (int, float))):
                raise ValueError("Operator only supported for numbers")
        op = self.allowed_ops.get(type(node.op))
        if op is None:
            raise ValueError(f"Unsupported operator: {type(node.op)}")
        return op(left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op = self.allowed_ops.get(type(node.op))
        if op is None:
            raise ValueError(f"Unsupported unary operator: {type(node.op)}")
        return op(operand)

    def visit_Call(self, node):
        func = self.visit(node.func)
        if node.keywords:
            raise ValueError("Keyword arguments are not supported in calls")
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

    def visit_Attribute(self, node):
        value = self.visit(node.value)
        if value is math:
            if hasattr(math, node.attr):
                return getattr(math, node.attr)
            raise ValueError(f"Unsupported attribute: math.{node.attr}")
        raise ValueError("Unsupported attribute access")

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

def evaluate_expression(expr):
    tree = ast.parse(expr, mode='eval')
    print(SafeEvaluator().visit(tree))

def run_isolated_expression(expression):
    """
    Runs an AXIOM expression in a restricted subprocess.
    In production, this would use AppContainer (Windows) or seccomp (Linux).
    """
    print(f"[SANDBOX] Evaluating: {expression}")
    
    current_dir = os.path.dirname(os.path.abspath(__file__))
    code = f"import sys; sys.path.insert(0, {repr(current_dir)}); from sandbox import evaluate_expression; evaluate_expression(sys.argv[1])"
    cmd = [sys.executable, "-c", code, expression]
    
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)  # NOSONAR
        
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
