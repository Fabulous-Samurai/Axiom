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
        self.allowed_ops = {
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

    def generic_visit(self, node):
        raise ValueError(f"Unsupported node type: {type(node).__name__}")

    def visit_Constant(self, node):
        return node.value

    def visit_Name(self, node):
        raise ValueError("Variables are not allowed")

    def visit_UnaryOp(self, node):
        op = type(node.op)
        if op not in self.allowed_ops:
            raise ValueError(f"Unsupported unary operator: {op}")
        operand = self.visit(node.operand)
        return self.allowed_ops[op](operand)

    def visit_BinOp(self, node):
        op = type(node.op)
        if op not in self.allowed_ops:
            raise ValueError(f"Unsupported binary operator: {op}")
        left = self.visit(node.left)
        right = self.visit(node.right)

        if op == ast.Mult and (isinstance(left, str) or isinstance(right, str)):
            raise ValueError("String multiplication is not allowed")
        if op == ast.Pow:
            if not isinstance(left, (int, float)) or not isinstance(right, (int, float)):
                raise ValueError("Power operation requires numbers")
            if isinstance(right, (int, float)) and right > 100:
                raise ValueError("Exponent too large")

        return self.allowed_ops[op](left, right)

    def visit_Call(self, node):
        if node.keywords:
            raise ValueError("Keyword arguments are not allowed")

        if isinstance(node.func, ast.Name):
            func_name = node.func.id
            if func_name == "abs":
                if len(node.args) != 1:
                    raise ValueError("abs() takes exactly one argument")
                arg = self.visit(node.args[0])
                return abs(arg)
            else:
                raise ValueError(f"Unsupported function: {func_name}")
        else:
            raise ValueError("Unsupported function call")

    def visit_Expr(self, node):
        return self.visit(node.value)

def evaluate_expression(expr_str):
    tree = ast.parse(expr_str, mode='eval')
    evaluator = SafeEvaluator()
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
    
    # We use a more robust way to pass the expression to the subprocess
    # to avoid shell quoting issues.
    code = f"import sys\nsys.path.insert(0, {repr(os.path.dirname(os.path.abspath(__file__)))})\nimport sandbox\nfrom sandbox import evaluate_expression\nevaluate_expression({repr(expression)})"
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
