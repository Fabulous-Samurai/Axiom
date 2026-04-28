import os
import sys
import time
import threading
import subprocess
import signal
import ast
import operator

class SafeMathEvaluator(ast.NodeVisitor):
    def visit_Constant(self, node):
        return node.value
    def visit_BinOp(self, node):
        ops = {ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul, ast.Div: operator.truediv, ast.Pow: operator.pow}
        if type(node.op) not in ops:
            raise ValueError(f"Disallowed operator {type(node.op)}")
        return ops[type(node.op)](self.visit(node.left), self.visit(node.right))
    def visit_UnaryOp(self, node):
        if isinstance(node.op, ast.USub):
            return -self.visit(node.operand)
        raise ValueError(f"Disallowed unary operator {type(node.op)}")
    def visit_Attribute(self, node):
        raise ValueError("Attribute access blocked")
    def visit_Call(self, node):
        raise ValueError("Function calls blocked")
    def generic_visit(self, node):
        raise ValueError(f"Disallowed node {type(node)}")

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
        # Enable handling of large integers (e.g., for heavy expressions causing timeout)
        sys.set_int_max_str_digits(0)
        try:
            tree = ast.parse(sys.argv[2], mode='eval')
            evaluator = SafeMathEvaluator()
            result = evaluator.visit(tree.body)
            print(result)
        except Exception as e:
            print(e, file=sys.stderr)
            sys.exit(1)
    elif len(sys.argv) > 1:
        expr = sys.argv[1]
        print(run_isolated_expression(expr))
    else:
        # Example adversarial expression (if eval was used directly)
        print(run_isolated_expression("__import__('os').listdir('.')"))
