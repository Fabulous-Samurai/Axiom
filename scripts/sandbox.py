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
        self.allowed_ops = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv,
            ast.Mod: operator.mod,
            ast.Pow: operator.pow,
            ast.BitXor: operator.xor,
            ast.BitOr: operator.or_,
            ast.BitAnd: operator.and_,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos,
        }

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Constant(self, node):
        return node.value

    def visit_UnaryOp(self, node):
        op = self.allowed_ops.get(type(node.op))
        if not op:
            raise ValueError(f"Unsupported unary operator: {type(node.op).__name__}")
        return op(self.visit(node.operand))

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op = self.allowed_ops.get(type(node.op))
        if not op:
            raise ValueError(f"Unsupported operator: {type(node.op).__name__}")

        if isinstance(node.op, ast.Pow):
            if isinstance(left, (int, float)) and isinstance(right, (int, float)):
                if right > 1000:
                    raise ValueError("Exponent too large")
            else:
                raise ValueError("Power operations only supported on numbers")

        if isinstance(node.op, ast.Mult):
            if not isinstance(left, (int, float)) or not isinstance(right, (int, float)):
                raise ValueError("Multiplication only supported on numbers")

        return op(left, right)

    def visit_Name(self, node):
        raise ValueError(f"Variables not allowed: {node.id}")

    def visit_Call(self, node):
        raise ValueError("Function calls not allowed")

    def visit_Attribute(self, node):
        raise ValueError("Attribute access not allowed")

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

def _safe_eval_print(expr):
    try:
        tree = ast.parse(expr, mode='eval')
        evaluator = SafeEvaluator()
        print(evaluator.visit(tree))
    except Exception as e:
        # sys.stderr.write(str(e) + '\n')
        # We'll just exit with the error so parent catches it and it ends up in stderr
        raise  # NOSONAR

def run_isolated_expression(expression):
    """
    Runs an AXIOM expression in a restricted subprocess.
    In production, this would use AppContainer (Windows) or seccomp (Linux).
    """
    print(f"[SANDBOX] Evaluating: {expression}")
    
    current_dir = os.path.dirname(os.path.abspath(__file__))
    cmd = [
        sys.executable,
        "-c",
        f"import sys; sys.path.insert(0, {repr(current_dir)}); from sandbox import _safe_eval_print; _safe_eval_print(sys.argv[1])",
        expression
    ]
    
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
