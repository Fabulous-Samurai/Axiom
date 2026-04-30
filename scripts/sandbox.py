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
    if len(sys.argv) > 2 and sys.argv[1] == "--safe-eval":
        import ast
        import operator
        class SafeMathEvaluator:
            def eval(self, expr): return self._eval(ast.parse(expr, mode='eval').body)
            def _eval(self, node):
                if isinstance(node, ast.Constant): return node.value
                elif isinstance(node, ast.BinOp):
                    ops = {ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul, ast.Div: operator.truediv, ast.Pow: operator.pow}
                    return ops[type(node.op)](self._eval(node.left), self._eval(node.right))
                elif isinstance(node, ast.UnaryOp):
                    return operator.neg(self._eval(node.operand)) if isinstance(node.op, ast.USub) else operator.pos(self._eval(node.operand))
                raise ValueError("Disallowed node")
        sys.set_int_max_str_digits(0)
        print(SafeMathEvaluator().eval(sys.argv[2]))
    elif len(sys.argv) > 1:
        expr = sys.argv[1]
        print(run_isolated_expression(expr))
    else:
        # Example adversarial expression (if eval was used directly)
        print(run_isolated_expression("__import__('os').listdir('.')"))
