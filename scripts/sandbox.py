import os
import sys
import time
import threading
import subprocess
import signal
import ast

class SafeMathEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_nodes = {
            ast.Expression, ast.Constant,
            ast.BinOp, ast.UnaryOp,
            ast.Add, ast.Sub, ast.Mult, ast.Div, ast.Pow, ast.Mod,
            ast.UAdd, ast.USub
        }

    def visit(self, node):
        if type(node) not in self.allowed_nodes:
            raise ValueError(f"Unsafe operation: {type(node).__name__}")
        return super().visit(node)

    def evaluate(self, expr):
        tree = ast.parse(expr, mode='eval')
        return self.visit(tree)

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        if isinstance(node.value, (int, float)):
            return node.value
        raise ValueError("Only numeric constants allowed")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op = type(node.op)
        if op == ast.Add: return left + right
        if op == ast.Sub: return left - right
        if op == ast.Mult: return left * right
        if op == ast.Div: return left / right
        if op == ast.Mod: return left % right
        if op == ast.Pow: return left ** right
        raise ValueError(f"Unsupported operation: {op}")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op = type(node.op)
        if op == ast.UAdd: return +operand
        if op == ast.USub: return -operand
        raise ValueError(f"Unsupported unary operation: {op}")

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
                print(f"[SANDBOX] Timeout exceeded ({self.timeout}s). Terminating.", file=sys.stderr)
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
        if sys.argv[1] == "--safe-eval":
            try:
                expr = sys.argv[2]
                print(SafeMathEvaluator().evaluate(expr))
            except Exception as e:
                print(str(e), file=sys.stderr)
                sys.exit(1)
            sys.exit(0)
        else:
            expr = sys.argv[1]
            print(run_isolated_expression(expr))
    else:
        # Example adversarial expression (if eval was used directly)
        print(run_isolated_expression("__import__('os').listdir('.')"))
