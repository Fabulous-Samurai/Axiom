with open("scripts/verify_zenith_pillars.py", "r") as f:
    content = f.read()

content = content.replace('    "python_repl.cpp"\n]', '    "python_repl.cpp",\n    "axiom_worker.cpp"\n]')

with open("scripts/verify_zenith_pillars.py", "w") as f:
    f.write(content)
