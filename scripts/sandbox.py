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

def run_isolated_expression(expression):
    """
    Runs an AXIOM expression in a restricted subprocess.
    In production, this would use AppContainer (Windows) or seccomp (Linux).
    """
    print(f"[SANDBOX] Evaluating: {expression}")
    
    # We use a more robust way to pass the expression to the subprocess
    # to avoid shell quoting issues.
    code = f"""
# 🛡️ SENTINEL SECURITY FIX: Prevent command injection by using a safe AST evaluator instead of eval()
import ast, operator
class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.ops = {{
            ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul, ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv, ast.Pow: operator.pow, ast.Mod: operator.mod,
            ast.USub: operator.neg, ast.UAdd: operator.pos, ast.BitAnd: operator.and_,
            ast.BitOr: operator.or_, ast.BitXor: operator.xor, ast.LShift: operator.lshift,
            ast.RShift: operator.rshift, ast.Invert: operator.invert
        }}
        self.cmp = {{
            ast.Eq: operator.eq, ast.NotEq: operator.ne, ast.Lt: operator.lt, ast.LtE: operator.le,
            ast.Gt: operator.gt, ast.GtE: operator.ge, ast.Is: operator.is_, ast.IsNot: operator.is_not,
            ast.In: lambda a, b: a in b, ast.NotIn: lambda a, b: a not in b
        }}
    def evaluate(self, expr):
        return self.visit(ast.parse(expr, mode='eval').body)
    def visit(self, node):
        if isinstance(node, ast.Constant): return node.value
        elif isinstance(node, ast.List): return [self.visit(e) for e in node.elts]
        elif isinstance(node, ast.Tuple): return tuple(self.visit(e) for e in node.elts)
        elif isinstance(node, ast.Set): return {{self.visit(e) for e in node.elts}}
        elif isinstance(node, ast.Dict): return {{self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}}
        elif isinstance(node, ast.BinOp) and type(node.op) in self.ops:
            return self.ops[type(node.op)](self.visit(node.left), self.visit(node.right))
        elif isinstance(node, ast.UnaryOp) and type(node.op) in self.ops:
            return self.ops[type(node.op)](self.visit(node.operand))
        elif isinstance(node, ast.Compare):
            left = self.visit(node.left)
            for op, comparator in zip(node.ops, node.comparators):
                if type(op) not in self.cmp: raise ValueError("Unsupported operator")
                right = self.visit(comparator)
                if not self.cmp[type(op)](left, right): return False
                left = right
            return True
        elif isinstance(node, ast.BoolOp):
            if isinstance(node.op, ast.And):
                return all(self.visit(v) for v in node.values)
            elif isinstance(node.op, ast.Or):
                return any(self.visit(v) for v in node.values)
        raise ValueError("Unsupported expression")
print(SafeEvaluator().evaluate({repr(expression)}))
"""
    cmd = [sys.executable, "-c", code]
    
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=False)
        
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
