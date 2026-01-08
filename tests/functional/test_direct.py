import subprocess
import sys

# Direct test with axiom executable
proc = subprocess.Popen(
    ["ninja-build/axiom.exe", "--interactive"],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    text=True,
    bufsize=0
)

commands = [
    "ln(10)",
    "log(10)",  
    "log2(8)",
    "sqrt(16)"
]

for cmd in commands:
    print(f"\nTesting: {cmd}")
    proc.stdin.write(cmd + "\n")
    proc.stdin.flush()
    
    output_lines = []
    while True:
        line = proc.stdout.readline()
        if "__END__" in line:
            break
        output_lines.append(line.rstrip())
    
    print(f"Result: {' '.join(output_lines)}")

proc.stdin.write("exit\n")
proc.stdin.flush()
proc.wait()
