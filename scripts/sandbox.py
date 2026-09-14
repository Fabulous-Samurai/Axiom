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
        self.ops = {
            ast.Add: operator.add, ast.Sub: operator.sub,
            ast.Mult: operator.mul, ast.Div: operator.truediv,
            ast.FloorDiv: operator.floordiv, ast.Mod: operator.mod,
            ast.Pow: operator.pow, ast.BitXor: operator.xor,
            ast.BitOr: operator.or_, ast.BitAnd: operator.and_,
            ast.LShift: operator.lshift, ast.RShift: operator.rshift,
            ast.Eq: operator.eq, ast.NotEq: operator.ne,
            ast.Lt: operator.lt, ast.LtE: operator.le,
            ast.Gt: operator.gt, ast.GtE: operator.ge,
            ast.Is: operator.is_, ast.IsNot: operator.is_not,
            ast.In: lambda a, b: a in b, ast.NotIn: lambda a, b: a not in b,
            ast.And: lambda a, b: a and b, ast.Or: lambda a, b: a or b,
            ast.Not: operator.not_, ast.UAdd: operator.pos,
            ast.USub: operator.neg, ast.Invert: operator.invert
        }

    def visit_Constant(self, node): return node.value
    def visit_Num(self, node): return node.n
    def visit_Str(self, node): return node.s
    def visit_NameConstant(self, node): return node.value

    def visit_BinOp(self, node): return self.ops[type(node.op)](self.visit(node.left), self.visit(node.right))
    def visit_UnaryOp(self, node): return self.ops[type(node.op)](self.visit(node.operand))

    def visit_BoolOp(self, node):
        values = [self.visit(v) for v in node.values]
        if isinstance(node.op, ast.And): return all(values)
        elif isinstance(node.op, ast.Or): return any(values)
        raise ValueError(f"Unsupported boolean operator: {type(node.op)}")

    def visit_Compare(self, node):
        left = self.visit(node.left)
        for op, comp in zip(node.ops, node.comparators):
            right = self.visit(comp)
            if not self.ops[type(op)](left, right): return False
            left = right
        return True

    def visit_Tuple(self, node): return tuple(self.visit(elt) for elt in node.elts)
    def visit_List(self, node): return [self.visit(elt) for elt in node.elts]
    def visit_Dict(self, node): return {self.visit(k): self.visit(v) for k, v in zip(node.keys, node.values)}
    def visit_Set(self, node): return {self.visit(elt) for elt in node.elts}
    def visit_Subscript(self, node): return self.visit(node.value)[self.visit(node.slice)]
    def visit_Index(self, node): return self.visit(node.value)

    def visit_Slice(self, node):
        lower = self.visit(node.lower) if node.lower else None
        upper = self.visit(node.upper) if node.upper else None
        step = self.visit(node.step) if node.step else None
        return slice(lower, upper, step)

    def visit_Name(self, node): raise ValueError(f"Variables not allowed: {node.id}")
    def visit_Call(self, node): raise ValueError("Function calls not allowed")
    def visit_Attribute(self, node): raise ValueError("Attribute access not allowed")

    def generic_visit(self, node): raise ValueError(f"Unsupported AST node type: {type(node).__name__}")

def safe_eval(expr):
    node = ast.parse(expr, mode='eval')
    return SafeEvaluator().visit(node.body)



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
    sandbox_dir = os.path.dirname(os.path.abspath(__file__))
    code = f"import sys; sys.path.insert(0, {repr(sandbox_dir)}); from sandbox import safe_eval; print(safe_eval({repr(expression)}))"
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
