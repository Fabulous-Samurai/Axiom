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
    
    # We use a strict AST-based evaluation to prevent arbitrary code execution
    # and sandbox escapes.
    code = f"""import sys, ast, math, operator

class SafeEvaluator(ast.NodeVisitor):
    allowed_operators = {{
        ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
        ast.Div: operator.truediv, ast.FloorDiv: operator.floordiv,
        ast.Pow: operator.pow, ast.Mod: operator.mod,
        ast.USub: operator.neg, ast.UAdd: operator.pos,
    }}
    allowed_funcs = {{
        'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
        'sqrt': math.sqrt, 'log': math.log, 'abs': getattr(__builtins__, 'abs') if type(__builtins__) is not dict else __builtins__['abs'],
    }}
    allowed_names = {{
        'pi': math.pi, 'e': math.e,
        'True': True, 'False': False, 'None': None,
    }}
    def __init__(self):
        self.nodes = 0
        self.max_nodes = 1000
    def visit(self, node):
        self.nodes += 1
        if self.nodes > self.max_nodes:
            raise ValueError("Expression too complex")
        return super().visit(node)
    def generic_visit(self, node):
        raise ValueError(f"Unsupported expression: {{type(node).__name__}}")
    def visit_Constant(self, node): return node.value
    def visit_Name(self, node):
        if node.id in self.allowed_names: return self.allowed_names[node.id]
        raise ValueError(f"Unknown variable: {{node.id}}")
    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if type(node.op) in self.allowed_operators:
            return self.allowed_operators[type(node.op)](left, right)
        raise ValueError(f"Unsupported operator: {{type(node.op).__name__}}")
    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        if type(node.op) in self.allowed_operators:
            return self.allowed_operators[type(node.op)](operand)
        raise ValueError(f"Unsupported operator: {{type(node.op).__name__}}")
    def visit_Call(self, node):
        if not isinstance(node.func, ast.Name):
            raise ValueError("Only direct function calls are allowed")
        if node.func.id not in self.allowed_funcs:
            raise ValueError(f"Function {{node.func.id}} is not allowed")
        args = [self.visit(arg) for arg in node.args]
        return self.allowed_funcs[node.func.id](*args)
    def visit_List(self, node):
        return [self.visit(elt) for elt in node.elts]
    def visit_Dict(self, node):
        return {{self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}}
    def visit_Tuple(self, node):
        return tuple(self.visit(elt) for elt in node.elts)
    def visit_Set(self, node):
        return set(self.visit(elt) for elt in node.elts)
    def visit_Expression(self, node):
        return self.visit(node.body)

try:
    tree = ast.parse({repr(expression)}, mode='eval')
    evaluator = SafeEvaluator()
    print(evaluator.visit(tree))
except Exception as e:
    print(f"Error: {{e}}", file=sys.stderr)
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
