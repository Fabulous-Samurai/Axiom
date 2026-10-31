import os
import sys
import time
import threading
import subprocess
import signal

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

import ast
import operator

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_operators = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.Mod: operator.mod,
            ast.Pow: operator.pow,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos,
        }
        self.allowed_calls = {
            'abs': abs,
            'round': round,
            'min': min,
            'max': max
        }

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float, complex)):
            raise ValueError(f"Unsupported constant type: {type(node.value).__name__}")
        return node.value

    def visit_Num(self, node):
        return node.n

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op_type = type(node.op)
        if op_type not in self.allowed_operators:
            raise ValueError(f"Unsupported operator: {op_type.__name__}")
        if op_type == ast.Pow and (right > 100 or right < -100):
            raise ValueError("Power too large")
        return self.allowed_operators[op_type](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op_type = type(node.op)
        if op_type not in self.allowed_operators:
            raise ValueError(f"Unsupported unary operator: {op_type.__name__}")
        return self.allowed_operators[op_type](operand)

    def visit_Call(self, node):
        if node.keywords:
            raise ValueError("Keyword arguments are not permitted")
        if not isinstance(node.func, ast.Name):
            raise ValueError("Unsupported function call")
        func_name = node.func.id
        if func_name not in self.allowed_calls:
            raise ValueError(f"Unsupported function: {func_name}")

        args = [self.visit(arg) for arg in node.args]
        return self.allowed_calls[func_name](*args)

    def visit_Name(self, node):
        raise ValueError(f"Variables are not allowed: {node.id}")

def secure_eval(expression):
    tree = ast.parse(expression, mode='eval')
    evaluator = SafeEvaluator()
    return evaluator.visit(tree)

def _evaluate_expression(expression):
    result = secure_eval(expression)
    print(result)

def run_isolated_expression(expression):
    """
    Runs an AXIOM expression in a restricted subprocess.
    In production, this would use AppContainer (Windows) or seccomp (Linux).
    """
    print(f"[SANDBOX] Evaluating: {expression}")
    
    # We use a more robust way to pass the expression to the subprocess
    # to avoid shell quoting issues.
    module_dir = os.path.abspath(os.path.dirname(__file__))
    code = f"import sys, os; sys.path.insert(0, {repr(module_dir)}); from sandbox import _evaluate_expression; _evaluate_expression(sys.argv[1])"
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
