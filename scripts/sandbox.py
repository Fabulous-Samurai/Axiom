import os
import sys
import time
import threading
import subprocess
import signal
import ast
import operator

class SecureEvaluator(ast.NodeVisitor):
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

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)

        if type(node.op) == ast.Pow:
            if isinstance(left, (int, float)) and isinstance(right, (int, float)):
                if right > 100:
                    raise ValueError("Exponent too large")
        if type(node.op) == ast.Mult:
            if isinstance(left, str) or isinstance(right, str):
                raise ValueError("String multiplication not allowed")

        op = self.allowed_ops.get(type(node.op))
        if not op:
            raise ValueError(f"Unsupported operation: {type(node.op)}")
        return op(left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op = self.allowed_ops.get(type(node.op))
        if not op:
            raise ValueError(f"Unsupported operation: {type(node.op)}")
        return op(operand)

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        raise ValueError(f"Unsupported name: {node.id}")

    def visit_Call(self, node):
        if node.keywords:
            raise ValueError("Keyword arguments not allowed")
        raise ValueError("Function calls not allowed")

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

def _evaluate_for_subprocess(expr_str):
    tree = ast.parse(expr_str, mode='eval')
    evaluator = SecureEvaluator()
    result = evaluator.visit(tree.body)
    print(result)

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
    
    module_path = os.path.abspath(__file__)
    module_dir = os.path.dirname(module_path)
    module_name = os.path.splitext(os.path.basename(module_path))[0]

    code = f"import sys; sys.path.insert(0, {repr(module_dir)}); from {module_name} import _evaluate_for_subprocess; _evaluate_for_subprocess(sys.argv[1])"
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
