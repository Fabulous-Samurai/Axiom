import subprocess
print("Diffs:")
print(subprocess.run(["git", "diff", "--name-only", "HEAD"], capture_output=True, text=True).stdout)
print("Uncommitted:")
print(subprocess.run(["git", "diff", "--name-only"], capture_output=True, text=True).stdout)
print("Cached:")
print(subprocess.run(["git", "diff", "--name-only", "--cached"], capture_output=True, text=True).stdout)
