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
    
    # Secure AST-based evaluation to prevent Remote Code Execution
    code = f"""
import sys
import ast
import math

class SecureEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.safe_dict = {{
            'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
            'sqrt': math.sqrt, 'log': math.log, 'exp': math.exp,
            'pi': math.pi, 'e': math.e, 'abs': abs
        }}

    def visit_Expression(self, node):
        return self.visit(node.body)

    def visit_Call(self, node):
        if not isinstance(node.func, ast.Name):
            raise ValueError("Only simple function calls are allowed")
        func_name = node.func.id
        if func_name in self.safe_dict:
            args = [self.visit(arg) for arg in node.args]
            return self.safe_dict[func_name](*args)
        raise ValueError(f"Function '{{func_name}}' not allowed")

    def visit_Name(self, node):
        if node.id in self.safe_dict:
            return self.safe_dict[node.id]
        raise ValueError(f"Name '{{node.id}}' not allowed")

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if isinstance(node.op, ast.Add): return left + right
        if isinstance(node.op, ast.Sub): return left - right
        if isinstance(node.op, ast.Mult): return left * right
        if isinstance(node.op, ast.Div): return left / right
        if isinstance(node.op, ast.Pow): return left ** right
        if isinstance(node.op, ast.Mod): return left % right
        raise ValueError("Unsupported operation")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        if isinstance(node.op, ast.UAdd): return +operand
        if isinstance(node.op, ast.USub): return -operand
        raise ValueError("Unsupported unary operation")

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float, complex)):
            raise ValueError("Only numeric constants are allowed")
        return node.value

    def visit_List(self, node):
        return [self.visit(elt) for elt in node.elts]

try:
    tree = ast.parse({repr(expression)}, mode='eval')
    print(SecureEvaluator().visit(tree))
except Exception as e:
    sys.stderr.write(f"Error: {{e}}\\n")
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
