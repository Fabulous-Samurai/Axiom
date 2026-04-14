with open(".pre-commit-config.yaml", "r") as f:
    content = f.read()
content = content.replace(
    "repo: https://github.com/pwaller/pre-commit-clang-format",
    "repo: https://github.com/pre-commit/mirrors-clang-format"
)
content = content.replace(
    "rev: v1.1.0",
    "rev: v19.1.1"
)
with open(".pre-commit-config.yaml", "w") as f:
    f.write(content)
