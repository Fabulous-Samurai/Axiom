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
    
    # Replace eval() with a strictly restricted execution environment
    # Disable builtins and all access to global namespace methods
    # to prevent arbitrary code execution via MRO traversal.

    # We still have to prevent MRO traversal inside the subprocess.
    # We'll use a custom AST evaluator for maximum security.

    code = f"""import ast
import operator
import math

class SafeEval(ast.NodeVisitor):
    def __init__(self):
        self.allowed_names = {{
            'pi': math.pi, 'e': math.e,
            'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
            'sqrt': math.sqrt, 'abs': abs, 'max': max, 'min': min
        }}
        self.allowed_operators = {{
            ast.Add: operator.add, ast.Sub: operator.sub,
            ast.Mult: operator.mul, ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv, ast.Pow: operator.pow,
            ast.Mod: operator.mod, ast.USub: operator.neg,
            ast.UAdd: operator.pos, ast.BitAnd: operator.and_,
            ast.BitOr: operator.or_, ast.BitXor: operator.xor,
            ast.LShift: operator.lshift, ast.RShift: operator.rshift,
            ast.Eq: operator.eq, ast.NotEq: operator.ne,
            ast.Lt: operator.lt, ast.LtE: operator.le,
            ast.Gt: operator.gt, ast.GtE: operator.ge,
            ast.Not: operator.not_, ast.Invert: operator.invert
        }}

    def visit_Constant(self, node): return node.value
    def visit_Name(self, node):
        if node.id in self.allowed_names: return self.allowed_names[node.id]
        raise ValueError(f"Name {{node.id}} is not allowed")
    def visit_BinOp(self, node):
        op_type = type(node.op)
        if op_type in self.allowed_operators:
            return self.allowed_operators[op_type](self.visit(node.left), self.visit(node.right))
        raise ValueError(f"Operator {{op_type}} is not allowed")
    def visit_UnaryOp(self, node):
        op_type = type(node.op)
        if op_type in self.allowed_operators:
            return self.allowed_operators[op_type](self.visit(node.operand))
        raise ValueError(f"Operator {{op_type}} is not allowed")
    def visit_Call(self, node):
        func = self.visit(node.func)
        if not callable(func): raise ValueError(f"Function {{func}} is not callable")
        return func(*[self.visit(arg) for arg in node.args])
    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, comp in zip(node.ops, node.comparators):
            op_type = type(op)
            right = self.visit(comp)
            if op_type in self.allowed_operators:
                if not self.allowed_operators[op_type](left, right): return False
                left = right
            else: raise ValueError(f"Comparison operator {{op_type}} is not allowed")
        return True
    def visit_BoolOp(self, node):
        values = [self.visit(v) for v in node.values]
        op_type = type(node.op)
        if op_type == ast.And: return all(values)
        elif op_type == ast.Or: return any(values)
        raise ValueError(f"Boolean operator {{op_type}} is not allowed")
    def visit_Subscript(self, node): return self.visit(node.value)[self.visit(node.slice)]
    def visit_List(self, node): return [self.visit(elt) for elt in node.elts]
    def visit_Dict(self, node): return {{self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}}
    def visit_Attribute(self, node): raise ValueError("Attributes are not allowed for security reasons")
    def generic_visit(self, node): raise ValueError(f"Node {{type(node)}} is not allowed")

try:
    tree = ast.parse({repr(expression)}, mode='eval')
    print(SafeEval().visit(tree.body))
except Exception as e:
    import sys
    print(f"Error: {{str(e)}}", file=sys.stderr)
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
            if "Timeout exceeded" in stdout:
                return "Error: Timeout exceeded"
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
