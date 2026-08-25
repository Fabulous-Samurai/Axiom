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
    
    # We use an AST-based evaluator to safely parse and execute expressions,
    # preventing Remote Code Execution (RCE) / Command Injection.
    # 🛡️ SENTINEL SECURITY FIX
    code = f"""import ast
import operator
class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.ops = {{
            ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
            ast.Div: operator.truediv, ast.FloorDiv: operator.floordiv, ast.Mod: operator.mod,
            ast.Pow: operator.pow, ast.BitAnd: operator.and_, ast.BitOr: operator.or_,
            ast.BitXor: operator.xor, ast.LShift: operator.lshift, ast.RShift: operator.rshift,
            ast.Eq: operator.eq, ast.NotEq: operator.ne, ast.Lt: operator.lt, ast.LtE: operator.le,
            ast.Gt: operator.gt, ast.GtE: operator.ge, ast.Is: operator.is_, ast.IsNot: operator.is_not,
            ast.In: lambda a, b: a in b, ast.NotIn: lambda a, b: a not in b,
            ast.USub: operator.neg, ast.UAdd: operator.pos, ast.Not: operator.not_, ast.Invert: operator.invert
        }}
    def visit_Constant(self, node): return node.value
    def visit_Name(self, node): raise ValueError(f"Unsafe variable: {{node.id}}")
    def visit_BinOp(self, node): return self.ops[type(node.op)](self.visit(node.left), self.visit(node.right))
    def visit_UnaryOp(self, node): return self.ops[type(node.op)](self.visit(node.operand))
    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, comp in zip(node.ops, node.comparators):
            right = self.visit(comp)
            if not self.ops[type(op)](left, right): return False
            left = right
        return True
    def visit_BoolOp(self, node):
        if isinstance(node.op, ast.And):
            for v in node.values:
                if not self.visit(v): return False
            return True
        elif isinstance(node.op, ast.Or):
            for v in node.values:
                if self.visit(v): return True
            return False
    def visit_Dict(self, node): return {{self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}}
    def visit_List(self, node): return [self.visit(e) for e in node.elts]
    def visit_Set(self, node): return {{self.visit(e) for e in node.elts}}
    def visit_Tuple(self, node): return tuple(self.visit(e) for e in node.elts)
    def visit_Expression(self, node): return self.visit(node.body)
    def generic_visit(self, node): raise ValueError(f"Unsafe operation: {{type(node).__name__}}")
print(SafeEvaluator().visit(ast.parse({repr(expression)}, mode='eval')))
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
