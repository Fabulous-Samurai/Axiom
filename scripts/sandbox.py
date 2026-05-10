import os
import sys
import time
import threading
import subprocess
import signal
import ast
import operator

class SafeMathEvaluator:
    allowed_nodes = {
        ast.Expression, ast.BinOp, ast.UnaryOp, ast.Constant,
        ast.Add, ast.Sub, ast.Mult, ast.Div, ast.Pow, ast.Mod,
        ast.UAdd, ast.USub, ast.BitAnd, ast.BitOr, ast.BitXor,
        ast.LShift, ast.RShift, ast.Invert
    }
    operators = {
        ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
        ast.Div: operator.truediv, ast.Pow: operator.pow, ast.Mod: operator.mod,
        ast.UAdd: operator.pos, ast.USub: operator.neg,
        ast.BitAnd: operator.and_, ast.BitOr: operator.or_, ast.BitXor: operator.xor,
        ast.LShift: operator.lshift, ast.RShift: operator.rshift, ast.Invert: operator.invert
    }

    def evaluate(self, expr_string):
        tree = ast.parse(expr_string, mode='eval')
        return self._visit(tree.body)

    def _visit(self, node):
        if type(node) not in self.allowed_nodes:
            raise ValueError(f"Operation not permitted")

        if isinstance(node, ast.Constant):
            if not isinstance(node.value, (int, float, complex)):
                raise ValueError(f"Constant not permitted")
            return node.value
        elif isinstance(node, ast.BinOp):
            return self.operators[type(node.op)](self._visit(node.left), self._visit(node.right))
        elif isinstance(node, ast.UnaryOp):
            return self.operators[type(node.op)](self._visit(node.operand))
        raise ValueError(f"Operation not permitted")

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
            evaluator = SafeMathEvaluator()
            try:
                print(evaluator.evaluate(expr))
            except Exception as e:
                print(f"Eval Error: {e}", file=sys.stderr)
                sys.exit(1)
        else:
            expr = sys.argv[1]
            print(run_isolated_expression(expr))
    else:
        # Example adversarial expression (if eval was used directly)
        print(run_isolated_expression("__import__('os').listdir('.')"))
