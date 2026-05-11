import os
import ast

def test_no_shell_true_in_scripts():
    """Verify that shell=True is not used in subprocess.run in scripts."""
    scripts_dir = "scripts"
    for filename in os.listdir(scripts_dir):
        if filename.endswith(".py"):
            filepath = os.path.join(scripts_dir, filename)
            with open(filepath, "r", encoding="utf-8") as f:
                content = f.read()
            tree = ast.parse(content)
            for node in ast.walk(tree):
                if isinstance(node, ast.Call):
                    is_subprocess_run = False
                    if isinstance(node.func, ast.Attribute):
                        if node.func.attr == "run" and getattr(node.func.value, "id", "") == "subprocess":
                            is_subprocess_run = True
                    if is_subprocess_run:
                        for kw in node.keywords:
                            if kw.arg == "shell" and isinstance(kw.value, ast.Constant) and kw.value.value is True:
                                assert False, f"shell=True found in {filepath} at line {node.lineno}"
