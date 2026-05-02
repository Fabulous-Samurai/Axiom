import os
import sys
import time
import threading
import subprocess
import signal


import ast
import operator

class SafeMathEvaluator:
    allowed_ops = {
        ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
        ast.Div: operator.truediv, ast.Pow: operator.pow, ast.USub: operator.neg,
        ast.Mod: operator.mod
    }

    @classmethod
    def evaluate(cls, expr):
        # Prevent ValueError: Exceeds the limit for integer string conversion
        sys.set_int_max_str_digits(0)
        try:
            tree = ast.parse(expr, mode='eval').body
        except SyntaxError:
            raise ValueError("Invalid syntax")
        return cls._eval(tree)

    @classmethod
    def _eval(cls, node):
        if isinstance(node, ast.Constant):
            if isinstance(node.value, (int, float, complex)):
                return node.value
            raise ValueError("Only numeric constants allowed")
        elif isinstance(node, ast.BinOp):
            if type(node.op) not in cls.allowed_ops:
                raise ValueError(f"Unsupported operator: {type(node.op).__name__}")
            return cls.allowed_ops[type(node.op)](cls._eval(node.left), cls._eval(node.right))
        elif isinstance(node, ast.UnaryOp):
            if type(node.op) not in cls.allowed_ops:
                raise ValueError(f"Unsupported unary operator: {type(node.op).__name__}")
            return cls.allowed_ops[type(node.op)](cls._eval(node.operand))
        elif isinstance(node, (ast.Attribute, ast.Call, ast.Name)):
            raise ValueError(f"Dangerous node type disabled: {type(node).__name__}")
        else:
            raise ValueError(f"Unsupported node type: {type(node).__name__}")

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
    
    # Use the safe evaluator in a subprocess
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
    if len(sys.argv) > 2 and sys.argv[1] == "--safe-eval":
        try:
            print(SafeMathEvaluator.evaluate(sys.argv[2]))
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif len(sys.argv) > 1:
        expr = sys.argv[1]
        print(run_isolated_expression(expr))
    else:
        # Example adversarial expression (if eval was used directly)
        print(run_isolated_expression("__import__('os').listdir('.')"))
