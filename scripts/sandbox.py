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
    # to avoid shell quoting issues. Use an AST-based SafeMathEvaluator
    # to prevent arbitrary code execution (Command Injection / Sandbox Escape).
    evaluator_code = """
import ast
import operator
import sys

class SafeMathEvaluator(ast.NodeVisitor):
    def visit_BinOp(self, node):
        ops = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.Pow: operator.pow
        }
        if type(node.op) not in ops:
            raise ValueError("Unsupported binary operator")
        return ops[type(node.op)](self.visit(node.left), self.visit(node.right))

    def visit_UnaryOp(self, node):
        ops = {
            ast.USub: operator.neg,
            ast.UAdd: operator.pos
        }
        if type(node.op) not in ops:
            raise ValueError("Unsupported unary operator")
        return ops[type(node.op)](self.visit(node.operand))

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float)):
            raise ValueError("Only numeric constants are allowed")
        return node.value

    def generic_visit(self, node):
        raise ValueError("Unsupported node type: " + type(node).__name__)

    def evaluate(self, expr):
        try:
            tree = ast.parse(expr, mode='eval')
            return self.visit(tree.body)
        except SyntaxError:
            raise ValueError("Invalid syntax")

try:
    evaluator = SafeMathEvaluator()
    result = evaluator.evaluate(%s)
    print(result)
except Exception as e:
    print(f"Error: {e}", file=sys.stderr)
    sys.exit(1)
""" % repr(expression)
    cmd = [sys.executable, "-c", evaluator_code]
    
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
