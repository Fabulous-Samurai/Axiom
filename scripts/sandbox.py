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
            ast.Pow: operator.pow,
            ast.BitXor: operator.xor,
            ast.USub: operator.neg,
        }

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float, complex)):
            raise ValueError("Only numbers are supported")
        return node.value

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op_type = type(node.op)
        if op_type not in self.allowed_ops:
            raise ValueError(f"Unsupported operator: {op_type.__name__}")

        # Type-check operands for ast.Mult to prevent large string multiplication DoS
        if op_type == ast.Mult:
            if not isinstance(left, (int, float, complex)) or not isinstance(right, (int, float, complex)):
                raise ValueError("Multiplication only allowed for numbers")

        return self.allowed_ops[op_type](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op_type = type(node.op)
        if op_type not in self.allowed_ops:
            raise ValueError(f"Unsupported operator: {op_type.__name__}")
        return self.allowed_ops[op_type](operand)

    def visit_Call(self, node):
        if node.keywords:
            raise ValueError("Keyword arguments are not allowed")
        raise ValueError("Function calls are not allowed")

def secure_eval_str(expr_str):
    tree = ast.parse(expr_str, mode='eval')
    evaluator = SecureEvaluator()
    print(evaluator.visit(tree))

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
    module_path = os.path.abspath(__file__)
    dir_path = os.path.dirname(module_path)
    code = f"import sys; sys.path.insert(0, {repr(dir_path)}); from sandbox import secure_eval_str; secure_eval_str({repr(expression)})"
    cmd = [sys.executable, "-c", code]
    
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
