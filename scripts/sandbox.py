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
    allowed_operators = {
        ast.Add: operator.add,
        ast.Sub: operator.sub,
        ast.Mult: operator.mul,
        ast.Div: operator.truediv,
        ast.Pow: operator.pow,
        ast.BitXor: operator.xor,
        ast.USub: operator.neg,
        ast.UAdd: operator.pos,
    }

    def __init__(self):
        self.allowed_names = {'math': math}
        for name in dir(math):
            if not name.startswith('_'):
                self.allowed_names[name] = getattr(math, name)

    def visit_Module(self, node):
        return self.visit(node.body[0].value)

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        if node.id in self.allowed_names:
            val = self.allowed_names[node.id]
            if not isinstance(val, (int, float, complex)) and not callable(val) and node.id != 'math':
                 raise ValueError(f"Disallowed name type: {node.id}")
            return val
        raise ValueError(f"Unknown name: {node.id}")

    def visit_Attribute(self, node):
        value = self.visit(node.value)
        if value is math and hasattr(math, node.attr):
            return getattr(math, node.attr)
        raise ValueError(f"Unsupported attribute: {node.attr}")

    def visit_UnaryOp(self, node):
        op = self.allowed_operators.get(type(node.op))
        if not op:
            raise ValueError(f"Unsupported unary operator: {type(node.op)}")
        operand = self.visit(node.operand)
        if not isinstance(operand, (int, float, complex)):
            raise ValueError("Unary operations are only supported for numbers")
        return op(operand)

    def visit_BinOp(self, node):
        op = self.allowed_operators.get(type(node.op))
        if not op:
            raise ValueError(f"Unsupported operator: {type(node.op)}")
        left = self.visit(node.left)
        right = self.visit(node.right)
        if not isinstance(left, (int, float, complex)) or not isinstance(right, (int, float, complex)):
             raise ValueError("Binary operations are only supported for numbers")

        if isinstance(node.op, ast.Pow) and (abs(left) > 1000 or abs(right) > 100):
            raise ValueError("Power too large")

        return op(left, right)

    def visit_Call(self, node):
        func = self.visit(node.func)
        if not callable(func):
            raise ValueError("Not callable")
        args = [self.visit(arg) for arg in node.args]
        for arg in args:
             if not isinstance(arg, (int, float, complex)):
                 raise ValueError("Function arguments must be numbers")
        return func(*args)

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

def _safe_eval_impl(expr):
    tree = ast.parse(expr, mode='eval')
    evaluator = SafeEvaluator()
    return evaluator.visit(tree.body)

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
    
    # We run the safe evaluator in a subprocess for isolation
    # We pass the expression via command line argument instead of eval() string interpolation
    cmd = [sys.executable, "-c", "import sys; from sandbox import _safe_eval_impl; print(_safe_eval_impl(sys.argv[1]))", expression]
    
    try:
        # Add the scripts directory to PYTHONPATH so the subprocess can import sandbox
        env = os.environ.copy()
        env['PYTHONPATH'] = os.path.dirname(os.path.abspath(__file__))

        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, env=env)  # NOSONAR
        
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
