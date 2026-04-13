import re

filepath = "scripts/verify_zenith_pillars.py"
with open(filepath, "r") as f:
    content = f.read()

diff_function = """
def get_modified_files():
    try:
        # Determine the base branch (defaulting to origin/main or origin/master if available)
        base_branch = os.environ.get('GITHUB_BASE_REF', 'main')
        if not base_branch:
            base_branch = 'main'

        # We just want files changed in the current PR/branch relative to the base branch.
        # But for robust CI, we might just use `git diff --name-only origin/main...HEAD`
        # Alternatively, let's just get the list of modified files using git diff --name-only HEAD~1 (or origin/main).
        # A simpler approach is to check if we are in a Git repo, and if so, find changed files.
        cmd = ['git', 'diff', '--name-only', 'origin/' + base_branch + '...HEAD']
        result = subprocess.run(cmd, capture_output=True, text=True, check=False)
        if result.returncode != 0:
            cmd = ['git', 'diff', '--name-only', 'HEAD~1']
            result = subprocess.run(cmd, capture_output=True, text=True, check=False)

        if result.returncode == 0:
            return set(f.strip() for f in result.stdout.splitlines() if f.strip())
    except Exception as e:
        print(f"[WARNING] Could not determine modified files: {e}")
    return None

"""

# Add subprocess
content = content.replace("import re\n", "import re\nimport subprocess\n")

# Add the function before verify_file
content = content.replace("def verify_file(file_path):", diff_function + "def verify_file(file_path):")

# Modify main loop
main_replacement = """
    modified_files = get_modified_files()

    for d in core_dirs:
        # Check if we are running from root or scripts dir
        search_path = d if os.path.exists(d) else os.path.join("..", d)
        if not os.path.exists(search_path): continue

        for root, _, files in os.walk(search_path):
            for file in files:
                # Tightened exemption check
                if any(exempt in file for exempt in EXEMPT_FILES) or file in WHITELISTED_FILES:
                    continue
                if file.endswith((".cpp", ".h", ".hpp", ".cc")):
                    path = os.path.join(root, file)

                    # Ensure path matches a modified file to avoid false positives on legacy code
                    if modified_files is not None:
                        normalized_path = os.path.normpath(path).replace('\\\\', '/')
                        if not any(f.endswith(normalized_path) for f in modified_files):
                            continue

                    violations = verify_file(path)
"""

content = re.sub(
    r'    for d in core_dirs:.*?violations = verify_file\(path\)',
    main_replacement.strip('\n'),
    content,
    flags=re.DOTALL
)

with open(filepath, "w") as f:
    f.write(content)
