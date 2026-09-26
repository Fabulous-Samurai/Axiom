import re

file_path = ".github/workflows/axiom_zenith_ci.yml"
with open(file_path, "r") as f:
    content = f.read()

content = re.sub(
    r"python -m pip install pre-commit",
    r"python -m pip install pre-commit==4.6.2",
    content
)

with open(file_path, "w") as f:
    f.write(content)
