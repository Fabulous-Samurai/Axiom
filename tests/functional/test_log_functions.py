import subprocess

proc = subprocess.Popen(
    ["ninja-build/axiom.exe", "--interactive"],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    text=True,
    bufsize=0
)

test_cases = [
    ("log(100)", "should be 2"),
    ("ln(2.71828)", "should be ~1"),
    ("lg(8)", "should be 3 (log base 2)"),
    ("log2(8)", "should be 3"),
    ("log2(16)", "should be 4"),
]

for cmd, desc in test_cases:
    print(f"\nTest: {cmd} ({desc})")
    proc.stdin.write(cmd + "\n")
    proc.stdin.flush()
    
    output = []
    while True:
        line = proc.stdout.readline()
        print(f"  Raw: '{line.rstrip()}'")
        if "__END__" in line:
            break
        if line.strip() and "Error" not in line:
            output.append(line.strip())
    
    if output:
        print(f"  ✅ Result: {output[0]}")
    else:
        print(f"  ❌ No result!")

proc.terminate()
