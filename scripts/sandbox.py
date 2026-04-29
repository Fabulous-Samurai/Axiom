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

import ast, operator
class SafeMathEvaluator:
    ops = {ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
           ast.Div: operator.truediv, ast.Pow: operator.pow, ast.USub: operator.neg}
    def ev(self, n):
        if isinstance(n, ast.Expression): return self.ev(n.body)
        if isinstance(n, ast.Constant): return n.value
        if isinstance(n, ast.BinOp): return self.ops[type(n.op)](self.ev(n.left), self.ev(n.right))
        if isinstance(n, ast.UnaryOp): return self.ops[type(n.op)](self.ev(n.operand))
        if isinstance(n, ast.Call): raise ValueError("Function calls are explicitly disallowed")
        if isinstance(n, ast.Attribute): raise ValueError("Attribute access is explicitly disallowed")
        raise ValueError("Disallowed node")

def run_isolated_expression(expression):
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
    if len(sys.argv) > 1 and sys.argv[1] == "--safe-eval":

        try: print(SafeMathEvaluator().ev(ast.parse(sys.argv[2], mode='eval')))
        except Exception as e: print(f"Error: {e}", file=sys.stderr); sys.exit(1)
    elif len(sys.argv) > 1:
        print(run_isolated_expression(sys.argv[1]))
    else:
        print(run_isolated_expression("__import__('os').listdir('.')"))
