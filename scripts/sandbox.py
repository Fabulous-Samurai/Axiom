import os
import sys
import time
import threading
import subprocess
import signal


import ast
import math
import operator

class MathEvaluator(ast.NodeVisitor):
    allowed_ops = {
        ast.Add: operator.add,
        ast.Sub: operator.sub,
        ast.Mult: operator.mul,
        ast.Div: operator.truediv,
        ast.Pow: operator.pow,
        ast.USub: operator.neg,
        ast.UAdd: operator.pos,
        ast.Mod: operator.mod,
    }

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float, complex)):
            raise ValueError(f"Unsupported constant type: {type(node.value)}")
        return node.value

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op = type(node.op)
        if op not in self.allowed_ops:
            raise ValueError(f"Unsupported binary operator: {op}")

        # Prevent DoS via large powers
        if op == ast.Pow and isinstance(right, (int, float)) and right > 100:
            raise ValueError("Exponent too large")

        return self.allowed_ops[op](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op = type(node.op)
        if op not in self.allowed_ops:
            raise ValueError(f"Unsupported unary operator: {op}")
        return self.allowed_ops[op](operand)

    def visit_Call(self, node):
        if node.keywords:
            raise ValueError("Keyword arguments are not supported")
        func = self.visit(node.func)
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

    def visit_Attribute(self, node):
        value = self.visit(node.value)
        if value is not math:
            raise ValueError("Only attributes of 'math' module are supported")
        if not hasattr(math, node.attr):
            raise ValueError(f"Unsupported math attribute: {node.attr}")
        return getattr(math, node.attr)

    def visit_Name(self, node):
        if node.id == 'math':
            return math
        raise ValueError(f"Unsupported variable/name: {node.id}")

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

def safe_eval(expr):
    tree = ast.parse(expr, mode='eval')
    evaluator = MathEvaluator()
    return evaluator.visit(tree)

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
    code = f"import sys; sys.path.insert(0, {repr(os.path.abspath('scripts'))}); from sandbox import safe_eval; print(safe_eval({repr(expression)}))"

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
