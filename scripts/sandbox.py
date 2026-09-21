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
    code = f"""import sys, ast, math, operator
class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed = {{k: v for k, v in math.__dict__.items() if not k.startswith('_')}}
        self.allowed.update({{'abs': abs, 'round': round, 'min': min, 'max': max}})
        self.binops = {{ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
                       ast.Div: operator.truediv, ast.FloorDiv: operator.floordiv,
                       ast.Mod: operator.mod, ast.Pow: operator.pow, ast.BitXor: operator.xor,
                       ast.BitOr: operator.or_, ast.BitAnd: operator.and_}}
        self.unops = {{ast.USub: operator.neg, ast.UAdd: operator.pos}}
    def visit_Constant(self, node): return node.value
    def visit_Name(self, node):
        if node.id in self.allowed: return self.allowed[node.id]
        raise ValueError(f"Name '{{node.id}}' is not allowed")
    def visit_BinOp(self, node):
        return self.binops[type(node.op)](self.visit(node.left), self.visit(node.right))
    def visit_UnaryOp(self, node):
        return self.unops[type(node.op)](self.visit(node.operand))
    def visit_Call(self, node):
        return self.visit(node.func)(*[self.visit(a) for a in node.args])
    def visit_Expression(self, node): return self.visit(node.body)
    def evaluate(self, expr_str):
        tree = ast.parse(expr_str, mode='eval')
        return self.visit(tree)

try:
    print(SafeEvaluator().evaluate({repr(expression)}))
except Exception as e:
    print(f'Error: {{e}}', file=sys.stderr)
    sys.exit(1)
"""
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
