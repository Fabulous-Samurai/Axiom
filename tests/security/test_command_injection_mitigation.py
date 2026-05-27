import ast
import os
import pytest

def check_for_shell_true(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        source = f.read()

    tree = ast.parse(source, filename=file_path)

    violations = []

    for node in ast.walk(tree):
        if isinstance(node, ast.Call):
            if isinstance(node.func, ast.Attribute) and node.func.attr == "run":
                # Check if it's subprocess.run
                is_subprocess_run = False
                if isinstance(node.func.value, ast.Name) and node.func.value.id == "subprocess":
                    is_subprocess_run = True

                if is_subprocess_run:
                    for keyword in node.keywords:
                        if keyword.arg == "shell":
                            if isinstance(keyword.value, ast.Constant) and keyword.value.value is True:
                                violations.append(f"Line {node.lineno}: subprocess.run with shell=True is forbidden.")

    return violations

def test_zenith_audit_scribe_no_shell_true():
    file_path = os.path.join(os.path.dirname(__file__), "..", "..", "scripts", "zenith_audit_scribe.py")
    violations = check_for_shell_true(file_path)
    assert not violations, f"Found shell=True in {file_path}:\n" + "\n".join(violations)

def test_sonar_helper_no_shell_true():
    file_path = os.path.join(os.path.dirname(__file__), "..", "..", "scripts", "sonar_helper.py")
    violations = check_for_shell_true(file_path)
    assert not violations, f"Found shell=True in {file_path}:\n" + "\n".join(violations)
