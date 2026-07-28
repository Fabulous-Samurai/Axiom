import sys

file_path = "scripts/zenith_audit_scribe.py"
with open(file_path, "r") as f:
    content = f.read()

content = content.replace("""        result = import shlex
        subprocess.run(shlex.split(cmd), shell=False, capture_output=True, text=True, timeout=300)""", """        import shlex
        result = subprocess.run(shlex.split(cmd), shell=False, capture_output=True, text=True, timeout=300)""")

with open(file_path, "w") as f:
    f.write(content)
