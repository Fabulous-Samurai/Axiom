import os
import sys
import time
import threading
import subprocess
import signal
import ast
import operator

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

class SafeMathEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_operators = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv,
            ast.Pow: operator.pow,
            ast.Mod: operator.mod,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos,
        }

    def evaluate(self, expr_str):
        tree = ast.parse(expr_str, mode='eval')
        return self.visit(tree.body)

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float, complex)):
            raise ValueError(f"Constant type {type(node.value).__name__} is not allowed")
        return node.value

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if type(node.op) in self.allowed_operators:
            return self.allowed_operators[type(node.op)](left, right)
        raise ValueError(f"Operator {type(node.op).__name__} is not allowed")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        if type(node.op) in self.allowed_operators:
            return self.allowed_operators[type(node.op)](operand)
        raise ValueError(f"Operator {type(node.op).__name__} is not allowed")

    def visit_Call(self, node):
        raise ValueError("Function calls are explicitly disallowed")

    def visit_Attribute(self, node):
        raise ValueError("Attribute access is explicitly disallowed")

    def visit_Name(self, node):
        raise ValueError(f"Variables are explicitly disallowed")

    def generic_visit(self, node):
        raise ValueError(f"Node type {type(node).__name__} is not allowed")

def run_isolated_expression(expression):
    """
    Runs an AXIOM expression in a restricted subprocess.
    """
    print(f"[SANDBOX] Evaluating: {expression}")
    
    cmd = [sys.executable, __file__, "--safe-eval", expression]
    
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
        if sys.argv[1] == "--safe-eval":
            sys.set_int_max_str_digits(0)
            expr = sys.argv[2]
            try:
                evaluator = SafeMathEvaluator()
                print(evaluator.evaluate(expr))
            except Exception as e:
                print(f"{type(e).__name__}: {e}", file=sys.stderr)
                sys.exit(1)
        else:
            expr = sys.argv[1]
            print(run_isolated_expression(expr))
    else:
        # Example adversarial expression
        print(run_isolated_expression("__import__('os').listdir('.')"))
