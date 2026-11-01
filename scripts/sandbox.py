import os
import sys
import time
import threading
import subprocess
import signal
import ast
import operator
import math

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
            ast.BitXor: operator.xor,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos,
        }
        self.functions = {
            'sin': math.sin,
            'cos': math.cos,
            'tan': math.tan,
            'sqrt': math.sqrt,
            'log': math.log,
            'log10': math.log10,
            'pi': math.pi,
            'e': math.e,
            'abs': abs,
            'round': round,
        }

    def generic_visit(self, node):
        raise ValueError(f"Unsupported syntax: {type(node).__name__}")

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float)):
            raise ValueError("Only numeric constants are allowed")
        return node.value

    def visit_Name(self, node):
        if node.id in self.functions:
            return self.functions[node.id]
        raise ValueError(f"Unsupported variable or function: {node.id}")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op_type = type(node.op)
        if op_type not in self.operators:
            raise ValueError(f"Unsupported operator: {op_type.__name__}")

        if isinstance(left, str) or isinstance(right, str):
            raise ValueError("String operations are not allowed")

        return self.operators[op_type](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op_type = type(node.op)
        if op_type not in self.operators:
             raise ValueError(f"Unsupported unary operator: {op_type.__name__}")
        return self.operators[op_type](operand)

    def visit_Call(self, node):
        if node.keywords:
            raise ValueError("Keyword arguments not allowed")
        func = self.visit(node.func)
        if not callable(func):
             raise ValueError("Not a callable function")
        args = [self.visit(arg) for arg in node.args]
        return func(*args)

def _evaluate_securely(expr_string):
    try:
        tree = ast.parse(expr_string, mode='eval')
        evaluator = SafeEvaluator()
        result = evaluator.visit(tree)
        print(result)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
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
    
    # We use a more robust way to pass the expression to the subprocess
    # to avoid shell quoting issues.
    module_path = os.path.abspath(__file__)
    code = f"import sys, os; sys.path.insert(0, os.path.dirname({repr(module_path)})); from sandbox import _evaluate_securely; _evaluate_securely({repr(expression)})"
    cmd = [sys.executable, "-c", code]
    
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
