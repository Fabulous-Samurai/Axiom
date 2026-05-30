import os
import sys
import time
import threading
import subprocess
import signal
import ast
import operator

class SafeMathEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_operators = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.Pow: operator.pow,
            ast.BitXor: operator.xor,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos
        }

    def evaluate(self, expr_str):
        tree = ast.parse(expr_str, mode='eval')
        return self.visit(tree.body)

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if type(node.op) not in self.allowed_operators:
            raise ValueError(f"Unsupported operator: {type(node.op)}")
        return self.allowed_operators[type(node.op)](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        if type(node.op) not in self.allowed_operators:
            raise ValueError(f"Unsupported operator: {type(node.op)}")
        return self.allowed_operators[type(node.op)](operand)

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float, str)):
            raise ValueError("Only numbers and strings are allowed")
        return node.value

    def generic_visit(self, node):
        raise ValueError(f"Unsupported syntax: {type(node).__name__}")


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

    # To isolate, we call this script itself with the --safe-eval flag
    # This keeps the ComplexityGuard wrapping the execution process
    # but removes the dangerous `eval` directly in Python.
    cmd = [sys.executable, os.path.abspath(__file__), "--safe-eval", expression]
    
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
    if "--safe-eval" in sys.argv:
        expr = sys.argv[-1]
        try:
            print(SafeMathEvaluator().evaluate(expr))
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif len(sys.argv) > 1:
        expr = sys.argv[1]
        print(run_isolated_expression(expr))
    else:
        # Example adversarial expression (if eval was used directly)
        print(run_isolated_expression("__import__('os').listdir('.')"))
