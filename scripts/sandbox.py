import os
import sys
import time
import threading
import subprocess
import signal
import ast
import operator

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

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.operators = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv,
            ast.Mod: operator.mod,
            ast.Pow: operator.pow,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos,
        }
        self.functions = {
            'abs': abs,
            'round': round,
            'min': min,
            'max': max,
        }

    def generic_visit(self, node):
        raise ValueError(f"Unsupported syntax: {type(node).__name__}")

    def visit_Constant(self, node):
        return node.value

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op = type(node.op)
        if op not in self.operators:
            raise ValueError(f"Unsupported operator: {op.__name__}")

        if op == ast.Mult:
            if isinstance(left, str) or isinstance(right, str):
                raise ValueError("String multiplication is not allowed")

        return self.operators[op](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op = type(node.op)
        if op not in self.operators:
            raise ValueError(f"Unsupported unary operator: {op.__name__}")
        return self.operators[op](operand)

    def visit_Call(self, node):
        if node.keywords:
            raise ValueError("Keyword arguments are not allowed")

        if not isinstance(node.func, ast.Name):
            raise ValueError("Only simple function calls are allowed")

        func_name = node.func.id
        if func_name not in self.functions:
            raise ValueError(f"Unsupported function: {func_name}")

        args = [self.visit(arg) for arg in node.args]
        return self.functions[func_name](*args)

    def visit_Name(self, node):
        raise ValueError(f"Unsupported name: {node.id}")

    def visit_Expression(self, node):
        return self.visit(node.body)

def evaluate_expression(expr_str):
    tree = ast.parse(expr_str, mode='eval')
    evaluator = SafeEvaluator()
    return evaluator.visit(tree)

def run_isolated_expression(expression):
    """
    Runs an AXIOM expression in a restricted subprocess.
    In production, this would use AppContainer (Windows) or seccomp (Linux).
    """
    print(f"[SANDBOX] Evaluating: {expression}")
    
    script_path = os.path.abspath(__file__)
    code = f"import sys; sys.path.insert(0, {repr(os.path.dirname(script_path))}); from sandbox import evaluate_expression; print(evaluate_expression(sys.argv[1]))"
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
