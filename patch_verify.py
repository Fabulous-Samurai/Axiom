with open("scripts/verify_zenith_pillars.py", "r") as f:
    content = f.read()

content = content.replace('if clean_line.startswith("//") or clean_line.startswith("/*") or clean_line.startswith("*"):', 'if clean_line.startswith("//") or clean_line.startswith("/*") or clean_line.startswith("*") or clean_line == "":')

with open("scripts/verify_zenith_pillars.py", "w") as f:
    f.write(content)
