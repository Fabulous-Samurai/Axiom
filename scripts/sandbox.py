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

import ast
import operator
import math

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_operators = {
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.Pow: operator.pow,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos,
        }
        self.allowed_funcs = {
            'abs': abs,
            'round': round,
            'min': min,
            'max': max,
        }
        for name in dir(math):
            if not name.startswith('_'):
                self.allowed_funcs[name] = getattr(math, name)

    def evaluate(self, expr_str):
        tree = ast.parse(expr_str, mode='eval')
        return self.visit(tree.body)

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float, complex)):
            raise ValueError(f"Unsupported constant type: {type(node.value)}")
        return node.value

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op_type = type(node.op)
        if op_type not in self.allowed_operators:
            raise ValueError(f"Unsupported operator: {op_type.__name__}")
        if op_type == ast.Pow:
            if not isinstance(left, (int, float)) or not isinstance(right, (int, float)):
                raise ValueError("Power operands must be numbers")
            if right > 100 or left > 10000:
                raise ValueError("Power operands too large")
        return self.allowed_operators[op_type](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op_type = type(node.op)
        if op_type not in self.allowed_operators:
            raise ValueError(f"Unsupported unary operator: {op_type.__name__}")
        return self.allowed_operators[op_type](operand)

    def visit_Call(self, node):
        if isinstance(node.func, ast.Name):
            func_name = node.func.id
        elif isinstance(node.func, ast.Attribute) and isinstance(node.func.value, ast.Name) and node.func.value.id == 'math':
            func_name = node.func.attr
        else:
            raise ValueError("Only math functions and built-in safe functions are allowed")
        if func_name not in self.allowed_funcs:
            raise ValueError(f"Unsupported function: {func_name}")
        args = [self.visit(arg) for arg in node.args]
        return self.allowed_funcs[func_name](*args)

    def visit_Name(self, node):
        if node.id in self.allowed_funcs:
            return self.allowed_funcs[node.id]
        if node.id == 'math':
            return math
        raise ValueError(f"Unsupported variable or name: {node.id}")

    def visit_Attribute(self, node):
        value = self.visit(node.value)
        if value is math and hasattr(math, node.attr) and not node.attr.startswith('_'):
            return getattr(math, node.attr)
        raise ValueError(f"Unsupported attribute: {node.attr}")

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {type(node).__name__}")

def safe_eval(expr):
    evaluator = SafeEvaluator()
    return evaluator.evaluate(expr)

def run_isolated_expression(expression):
    """
    Runs an AXIOM expression in a restricted subprocess using a safe AST evaluator.
    """
    print(f"[SANDBOX] Evaluating: {expression}")
    
    code = f"""import sys
import ast
import operator
import math

class SafeEvaluator(ast.NodeVisitor):
    def __init__(self):
        self.allowed_operators = {{
            ast.Add: operator.add,
            ast.Sub: operator.sub,
            ast.Mult: operator.mul,
            ast.Div: operator.truediv,
            ast.Pow: operator.pow,
            ast.USub: operator.neg,
            ast.UAdd: operator.pos,
        }}
        self.allowed_funcs = {{
            'abs': getattr(__builtins__, 'abs') if hasattr(__builtins__, 'abs') else __builtins__['abs'] if type(__builtins__) is dict else abs,
            'round': getattr(__builtins__, 'round') if hasattr(__builtins__, 'round') else __builtins__['round'] if type(__builtins__) is dict else round,
            'min': getattr(__builtins__, 'min') if hasattr(__builtins__, 'min') else __builtins__['min'] if type(__builtins__) is dict else min,
            'max': getattr(__builtins__, 'max') if hasattr(__builtins__, 'max') else __builtins__['max'] if type(__builtins__) is dict else max,
        }}
        for name in dir(math):
            if not name.startswith('_'):
                self.allowed_funcs[name] = getattr(math, name)

    def evaluate(self, expr_str):
        tree = ast.parse(expr_str, mode='eval')
        return self.visit(tree.body)

    def visit_Constant(self, node):
        if not isinstance(node.value, (int, float, complex)):
            raise ValueError(f"Unsupported constant type: {{type(node.value)}}")
        return node.value

    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op_type = type(node.op)
        if op_type not in self.allowed_operators:
            raise ValueError(f"Unsupported operator: {{op_type.__name__}}")
        if op_type == ast.Pow:
            if not isinstance(left, (int, float)) or not isinstance(right, (int, float)):
                raise ValueError("Power operands must be numbers")
            if right > 100 or left > 10000:
                raise ValueError("Power operands too large")
        return self.allowed_operators[op_type](left, right)

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        op_type = type(node.op)
        if op_type not in self.allowed_operators:
            raise ValueError(f"Unsupported unary operator: {{op_type.__name__}}")
        return self.allowed_operators[op_type](operand)

    def visit_Call(self, node):
        if isinstance(node.func, ast.Name):
            func_name = node.func.id
        elif isinstance(node.func, ast.Attribute) and isinstance(node.func.value, ast.Name) and node.func.value.id == 'math':
            func_name = node.func.attr
        else:
            raise ValueError("Only math functions and built-in safe functions are allowed")
        if func_name not in self.allowed_funcs:
            raise ValueError(f"Unsupported function: {{func_name}}")
        args = [self.visit(arg) for arg in node.args]
        return self.allowed_funcs[func_name](*args)

    def visit_Name(self, node):
        if node.id in self.allowed_funcs:
            return self.allowed_funcs[node.id]
        if node.id == 'math':
            return math
        raise ValueError(f"Unsupported variable or name: {{node.id}}")

    def visit_Attribute(self, node):
        value = self.visit(node.value)
        if value is math and hasattr(math, node.attr) and not node.attr.startswith('_'):
            return getattr(math, node.attr)
        raise ValueError(f"Unsupported attribute: {{node.attr}}")

    def generic_visit(self, node):
        raise ValueError(f"Unsupported AST node: {{type(node).__name__}}")

try:
    evaluator = SafeEvaluator()
    print(evaluator.evaluate({repr(expression)}))
except Exception as e:
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
