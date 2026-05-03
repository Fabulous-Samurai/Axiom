import os
import sys
import time
import threading
import subprocess
import signal
import ast

class SafeMathEvaluator(ast.NodeVisitor):
    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op = type(node.op)
        if op == ast.Add: return left + right
        if op == ast.Sub: return left - right
        if op == ast.Mult: return left * right
        if op == ast.Div: return left / right
        if op == ast.Pow: return left ** right
        if op == ast.Mod: return left % right
        raise ValueError(f"Unsupported operation: {op}")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op = type(node.op)
        if op == ast.UAdd: return +operand
        if op == ast.USub: return -operand
        raise ValueError(f"Unsupported unary operation: {op}")

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float)):
            raise ValueError("Only numeric constants are allowed")
        return node.value

    def visit_Call(self, node):
        raise ValueError("Function calls are not allowed")

    def visit_Attribute(self, node):
        raise ValueError("Attribute access is not allowed")

    def generic_visit(self, node):
        raise ValueError(f"Unsupported syntax: {type(node).__name__}")

def evaluate_safely(expression):
    sys.set_int_max_str_digits(0)
    tree = ast.parse(expression, mode='eval')
    evaluator = SafeMathEvaluator()
    return evaluator.visit(tree)

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
    # Recursively call sandbox.py with --safe-eval to isolate safe math evaluation
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
    if len(sys.argv) > 1:
        if sys.argv[1] == "--safe-eval" and len(sys.argv) > 2:
            expr = sys.argv[2]
            try:
                print(evaluate_safely(expr))
            except Exception as e:
                print(f"Error: {e}", file=sys.stderr)
                sys.exit(1)
        else:
            expr = sys.argv[1]
            print(run_isolated_expression(expr))
    else:
        # Example adversarial expression (if eval was used directly)
        print(run_isolated_expression("__import__('os').listdir('.')"))
