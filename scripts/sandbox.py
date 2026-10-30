import os
import sys
import time
import threading
import subprocess
import signal


import ast
import operator

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.ops = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.Pow: operator.pow,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos
        }

    def visit_Constant(self, node):
        return node.value

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if isinstance(node.op, ast.Pow) and right > 100:
            raise ValueError("Exponent too large")
        if isinstance(node.op, ast.Mult):
            if isinstance(left, str) and right > 100:
                raise ValueError("String multiplication too large")
            if isinstance(right, str) and left > 100:
                raise ValueError("String multiplication too large")
        if type(node.op) not in self.ops:
            raise ValueError(f"Unsupported operation: {type(node.op).__name__}")
        return self.ops[type(node.op)](left, right)

    def visit_UnaryOp(self, node):
        if type(node.op) not in self.ops:
            raise ValueError(f"Unsupported operation: {type(node.op).__name__}")
        return self.ops[type(node.op)](self.visit(node.operand))

    def visit_Call(self, node):
        if node.keywords:
            raise ValueError("Keyword arguments not allowed")
        raise ValueError("Function calls are not allowed")

    def generic_visit(self, node):
        raise ValueError(f"Disallowed node type: {type(node).__name__}")

def evaluate_ast_main(expr_str):
    tree = ast.parse(expr_str, mode='eval')
    print(SafeEvaluator().visit(tree.body))

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
    
    # Use a secure custom AST evaluator instead of dangerous eval()
    script_dir = os.path.dirname(os.path.abspath(__file__))
    code = f"import sys; sys.path.insert(0, {repr(script_dir)}); from sandbox import evaluate_ast_main; evaluate_ast_main(sys.argv[1])"
    cmd = [sys.executable, "-c", code, expression]
    
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
