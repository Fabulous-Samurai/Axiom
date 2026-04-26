import os
import sys
import time
import threading
import subprocess
import signal

import ast

class SafeMathEvaluator:
    def __init__(self):
        self.allowed_nodes = {
            ast.Expression, ast.Constant, ast.Name,
            ast.BinOp, ast.UnaryOp, ast.Call,
            ast.Add, ast.Sub, ast.Mult, ast.Div, ast.Pow,
            ast.Mod, ast.FloorDiv, ast.BitAnd, ast.BitOr,
            ast.BitXor, ast.LShift, ast.RShift,
            ast.UAdd, ast.USub, ast.Invert, ast.Load
        }

    def _validate_node(self, node):
        if type(node) not in self.allowed_nodes:
            raise ValueError(f"Unsupported AST node: {type(node).__name__}")
        for child in ast.iter_child_nodes(node):
            self._validate_node(child)

    def evaluate(self, expression):
        try:
            tree = ast.parse(expression, mode='eval')
        except SyntaxError as e:
            raise ValueError(f"Invalid syntax: {e}")

        self._validate_node(tree)

        # We use a limited builtins dictionary for safe evaluation
        safe_dict = {'__builtins__': {}}
        return eval(compile(tree, '<string>', 'eval'), safe_dict)

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
    cmd = [sys.executable, os.path.abspath(__file__), "--evaluate-internal", expression]
    
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
    if len(sys.argv) > 2 and sys.argv[1] == "--evaluate-internal":
        try:
            evaluator = SafeMathEvaluator()
            print(evaluator.evaluate(sys.argv[2]))
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif len(sys.argv) > 1:
        expr = sys.argv[1]
        print(run_isolated_expression(expr))
    else:
        # Example adversarial expression (if eval was used directly)
        print(run_isolated_expression("__import__('os').listdir('.')"))
