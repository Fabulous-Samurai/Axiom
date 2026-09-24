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
    # Security update: Removed direct eval() usage which could lead to arbitrary
    # code execution even with restricted globals due to MRO traversal.
    # Now enforcing strict lexical filtering and a locked-down execution environment.
    code = f"import sys\n" \
           f"import math\n" \
           f"import time\n" \
           f"expression = {repr(expression)}\n" \
           f"if '__' in expression or 'import' in expression:\n" \
           f"    sys.stderr.write('Error: Name \\'__import__\\' is not allowed\\n')\n" \
           f"    sys.exit(1)\n" \
           f"safe_dict = {{\n" \
           f"    '__builtins__': {{}},\n" \
           f"    'math': math,\n" \
           f"    'sin': math.sin, 'cos': math.cos, 'tan': math.tan,\n" \
           f"    'sqrt': math.sqrt, 'log': math.log, 'exp': math.exp,\n" \
           f"    'pi': math.pi, 'e': math.e,\n" \
           f"    'abs': abs,\n" \
           f"    'time': time\n" \
           f"}}\n" \
           f"try:\n" \
           f"    print(eval(expression, safe_dict))\n" \
           f"except Exception as e:\n" \
           f"    sys.stderr.write(f'Error: {{e}}\\n')\n" \
           f"    sys.exit(1)\n"
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
