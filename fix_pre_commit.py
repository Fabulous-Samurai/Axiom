with open(".pre-commit-config.yaml", "r") as f:
    content = f.read()

new_content = content.replace(
    "repo: https://github.com/pwaller/pre-commit-clang-format\n    rev: v1.1.0",
    "repo: https://github.com/pre-commit/mirrors-clang-format\n    rev: v18.1.8"
)

with open(".pre-commit-config.yaml", "w") as f:
    f.write(new_content)
