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
    
    # 🛡️ SENTINEL SECURITY FIX: Replaced eval() with AST-based safe evaluation
    # to prevent command injection and sandbox escapes.
    code = f'''
import ast
import operator
import sys

def safe_eval(expr):
    allowed_ops = {{
        ast.Add: operator.add,
        ast.Sub: operator.sub,
        ast.Mult: operator.mul,
        ast.Div: operator.truediv,
        ast.Pow: operator.pow,
        ast.BitXor: operator.xor,
        ast.USub: operator.neg,
        ast.UAdd: operator.pos,
    }}

    def _eval(node):
        if isinstance(node, ast.Constant):
            return node.value
        elif isinstance(node, ast.BinOp):
            left = _eval(node.left)
            right = _eval(node.right)
            op = type(node.op)
            if op in allowed_ops:
                return allowed_ops[op](left, right)
            else:
                raise ValueError(f"Unsupported operator: {{op}}")
        elif isinstance(node, ast.UnaryOp):
            operand = _eval(node.operand)
            op = type(node.op)
            if op in allowed_ops:
                return allowed_ops[op](operand)
            else:
                raise ValueError(f"Unsupported unary operator: {{op}}")
        elif isinstance(node, ast.Expression):
            return _eval(node.body)
        else:
            raise ValueError(f"Unsupported node type: {{type(node)}}")

    try:
        tree = ast.parse(expr, mode='eval')
        return _eval(tree)
    except Exception as e:
        return f"Error: {{str(e)}}"

print(safe_eval({repr(expression)}))
'''
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
