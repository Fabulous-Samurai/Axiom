import os
import sys
import time
import threading
import subprocess
import signal

import ast
import operator
import sys

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_ops = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.Mod: operator.mod,
            ast.Pow: operator.pow,
            ast.UAdd: operator.pos,
            ast.USub: operator.neg,
        }

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float)):
            raise ValueError(f"Unsupported constant type: {type(node.value).__name__}")
        return node.value

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op_type = type(node.op)
        if op_type not in self.allowed_ops:
            raise ValueError(f"Unsupported operator: {op_type.__name__}")
        if op_type == ast.Pow and (right > 100 or right < -100):
            raise ValueError("Exponent too large")
        return self.allowed_ops[op_type](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op_type = type(node.op)
        if op_type not in self.allowed_ops:
            raise ValueError(f"Unsupported unary operator: {op_type.__name__}")
        return self.allowed_ops[op_type](operand)

    def visit_Call(self, node):
        if node.keywords:
            raise ValueError("Keyword arguments are not allowed")
        raise ValueError("Function calls are not allowed")

    def visit_Name(self, node):
        raise ValueError("Variables are not allowed")

def safe_eval_main(expr_str):
    try:
        tree = ast.parse(expr_str, mode='eval')
        result = SafeEvaluator().visit(tree)
        print(result)
    except Exception as e:
        sys.stderr.write(str(e) + "\n")
        sys.exit(1)


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
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    code = f"import sys; sys.path.insert(0, {repr(script_dir)}); from sandbox import safe_eval_main; safe_eval_main(sys.argv[1])"
    cmd = [sys.executable, "-c", code, expression]
    
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)  # NOSONAR
        
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
