import sys

with open('scripts/sandbox.py', 'r') as f:
    content = f.read()

search = """    code = (
        "import sys\\n"
        "safe_dict = {'__builtins__': {'abs': abs, 'min': min, 'max': max, 'int': int, 'float': float}}\\n"
        "try:\\n"
        "    print(eval(%r, safe_dict))\\n"
        "except Exception as e:\\n"
        "    print(str(e), file=sys.stderr)\\n"
        "    sys.exit(1)"
    ) % expression
    cmd = [sys.executable, "-c", code]

    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)"""

replace = """    code = (
        "import sys\\n"
        "safe_dict = {'__builtins__': {'abs': abs, 'min': min, 'max': max, 'int': int, 'float': float}}\\n"
        "try:\\n"
        "    print(eval(%r, safe_dict))\\n"
        "except Exception as e:\\n"
        "    print(str(e), file=sys.stderr)\\n"
        "    sys.exit(1)"
    ) % expression

    import shutil
    executable = shutil.which(sys.executable) or sys.executable
    cmd = [executable, "-c", code]

    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=False)"""

if search in content:
    content = content.replace(search, replace)
    print("Replaced!")
else:
    print("Not found")

with open('scripts/sandbox.py', 'w') as f:
    f.write(content)
