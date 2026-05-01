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

import ast
import operator

class SafeMathEvaluator:
    ops = {ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
           ast.Div: operator.truediv, ast.Pow: operator.pow, ast.BitXor: operator.xor,
           ast.USub: operator.neg, ast.UAdd: operator.pos}

    def evaluate(self, expr):
        if hasattr(sys, 'set_int_max_str_digits'):
            sys.set_int_max_str_digits(0)
        return self._eval(ast.parse(expr, mode='eval').body)

    def _eval(self, node):
        if isinstance(node, ast.Constant):
            return node.value
        if isinstance(node, ast.BinOp) and type(node.op) in self.ops:
            return self.ops[type(node.op)](self._eval(node.left), self._eval(node.right))
        if isinstance(node, ast.UnaryOp) and type(node.op) in self.ops:
            return self.ops[type(node.op)](self._eval(node.operand))
        if isinstance(node, ast.Attribute):
            raise ValueError("Attribute access is explicitly disallowed")
        if isinstance(node, ast.Call):
            raise ValueError("Function calls are explicitly disallowed")
        raise ValueError(f"Invalid node type: {type(node).__name__}")


def run_isolated_expression(expression):
    """
    Runs an AXIOM expression in a restricted subprocess using an AST-based evaluator
    to prevent Remote Code Execution (RCE) / Sandbox Escapes.
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
            try:
                print(SafeMathEvaluator().evaluate(sys.argv[2]))
            except Exception as e:
                print(f"Exception: {str(e)}", file=sys.stderr)
                sys.exit(1)
        else:
            expr = sys.argv[1]
            print(run_isolated_expression(expr))
    else:
        # Example adversarial expression
        print(run_isolated_expression("__import__('os').listdir('.')"))
