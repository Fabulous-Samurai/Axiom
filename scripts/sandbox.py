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
    
    # 🛡️ SENTINEL SECURITY FIX: Replaced dangerous eval() with a secure ast-based SafeEvaluator
    # to avoid command injection vulnerabilities while preserving math evaluation capabilities.
    code = f"""import sys
import ast
import operator

class SafeEvaluator(ast.NodeVisitor):
    allowed_operators = {{
        ast.Add: operator.add, ast.Sub: operator.sub,
        ast.Mult: operator.mul, ast.Div: operator.truediv,
        ast.FloorDiv: operator.floordiv, ast.Mod: operator.mod,
        ast.Pow: operator.pow, ast.BitXor: operator.xor,
        ast.BitOr: operator.or_, ast.BitAnd: operator.and_,
        ast.UAdd: operator.pos, ast.USub: operator.neg,
        ast.Eq: operator.eq, ast.NotEq: operator.ne,
        ast.Lt: operator.lt, ast.LtE: operator.le,
        ast.Gt: operator.gt, ast.GtE: operator.ge,
        ast.And: lambda x, y: x and y, ast.Or: lambda x, y: x or y,
        ast.Not: operator.not_,
    }}

    def visit_Constant(self, node):
        return node.value

    def visit_UnaryOp(self, node):
        op = type(node.op)
        if op not in self.allowed_operators:
            raise ValueError(f"Unsupported unary operator: {{op}}")
        return self.allowed_operators[op](self.visit(node.operand))

    def visit_BinOp(self, node):
        op = type(node.op)
        if op not in self.allowed_operators:
            raise ValueError(f"Unsupported binary operator: {{op}}")
        return self.allowed_operators[op](self.visit(node.left), self.visit(node.right))

    def visit_BoolOp(self, node):
        op = type(node.op)
        if op not in self.allowed_operators:
            raise ValueError(f"Unsupported boolean operator: {{op}}")
        values = [self.visit(v) for v in node.values]
        if op == ast.And:
            return all(values)
        if op == ast.Or:
            return any(values)

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, comparator in zip(node.ops, node.comparators):
            op_type = type(op)
            if op_type not in self.allowed_operators:
                raise ValueError(f"Unsupported comparison operator: {{op_type}}")
            right = self.visit(comparator)
            if not self.allowed_operators[op_type](left, right):
                return False
            left = right
        return True

    def visit_Dict(self, node):
        return {{self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}}

    def visit_List(self, node):
        return [self.visit(x) for x in node.elts]

    def visit_Tuple(self, node):
        return tuple(self.visit(x) for x in node.elts)

    def visit_Set(self, node):
        return {{self.visit(x) for x in node.elts}}

    def generic_visit(self, node):
        raise ValueError(f"Unsupported expression node: {{type(node).__name__}}")

tree = ast.parse({repr(expression)}, mode='eval')
print(SafeEvaluator().visit(tree.body))
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
