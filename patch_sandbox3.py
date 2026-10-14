with open("scripts/sandbox.py", "r") as f:
    content = f.read()

content = content.replace("code = f\"\"\"\nimport ast", "code = f\"\"\"\nimport ast\nimport sys")
content = content.replace("import sys\n    print(str(e), file=sys.stderr)\n    sys.exit(1)", "print(str(e), file=sys.stderr)\n    sys.exit(1)")

with open("scripts/sandbox.py", "w") as f:
    f.write(content)
