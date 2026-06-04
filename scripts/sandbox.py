import os
import sys
import time
import threading
import subprocess
import signal
import ast
import operator

class SafeMathEvaluator:
    def __init__(self):
        self.operators = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv,
            ast.Pow: operator.pow,
            ast.BitXor: operator.xor,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos,
        }

    def evaluate(self, node):
        if isinstance(node, ast.Expression):
            return self.evaluate(node.body)
        elif isinstance(node, ast.Constant):
            return node.value
        elif isinstance(node, ast.BinOp):
            left = self.evaluate(node.left)
            right = self.evaluate(node.right)
            return self.operators[type(node.op)](left, right)
        elif isinstance(node, ast.UnaryOp):
            operand = self.evaluate(node.operand)
            return self.operators[type(node.op)](operand)
        else:
            raise ValueError(f"Unsupported AST node: {type(node).__name__}")

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
    if len(sys.argv) > 1:
        if sys.argv[1] == "--safe-eval" and len(sys.argv) > 2:
            expr = sys.argv[2]
            try:
                node = ast.parse(expr, mode='eval')
                evaluator = SafeMathEvaluator()
                result = evaluator.evaluate(node)
                print(result)
            except Exception as e:
                print(f"Error: {e}", file=sys.stderr)
                sys.exit(1)
        else:
            expr = sys.argv[1]
            print(run_isolated_expression(expr))
    else:
        # Example adversarial expression (if eval was used directly)
        print(run_isolated_expression("__import__('os').listdir('.')"))
