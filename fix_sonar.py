import subprocess
cmd = ["code", "--goto", f"file.txt:1"]
print(f"Executing: {' '.join(cmd)}")
subprocess.run(cmd, check=True, shell=False)
