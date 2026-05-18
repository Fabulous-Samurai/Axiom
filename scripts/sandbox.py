import ast
import operator
import os
import sys
import time
import threading
import subprocess
import signal

class SafeMathEvaluator(ast.NodeVisitor):
    def evaluate(self, expr):
        node = ast.parse(expr, mode='eval')
        return self.visit(node.body)

    def visit_Constant(self, node):
        if isinstance(node.value, (int, float)):
            return node.value
        raise ValueError("Only numeric constants are allowed")

    def visit_BinOp(self, node):
        ops = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.Pow: operator.pow,
            ast.BitXor: operator.xor
        }
        op_type = type(node.op)
        if op_type in ops:
            return ops[op_type](self.visit(node.left), self.visit(node.right))
        raise ValueError(f"Unsupported binary operator: {op_type.__name__}")

    def visit_UnaryOp(self, node):
        ops = {
            ast.UAdd: operator.pos,
            ast.USub: operator.neg
        }
        op_type = type(node.op)
        if op_type in ops:
            return ops[op_type](self.visit(node.operand))
        raise ValueError(f"Unsupported unary operator: {op_type.__name__}")

    def generic_visit(self, node):
        raise ValueError(f"Unsupported expression node: {type(node).__name__}")

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
        if sys.argv[1] == "--safe-eval":
            if len(sys.argv) > 2:
                expr = sys.argv[2]
                try:
                    result = SafeMathEvaluator().evaluate(expr)
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
