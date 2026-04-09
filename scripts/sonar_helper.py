import json
import os
import argparse
import subprocess
import sys

def list_issues(issues):
    print(f"{'#':<3} | {'Severity':<10} | {'Type':<12} | {'File:Line':<40} | {'Message'}")
    print("-" * 100)
    for i, issue in enumerate(issues):
        severity = issue.get("severity", "N/A")
        type_ = issue.get("type", "N/A")
        component = issue.get("component", "").split(":")[-1]
        line = issue.get("line", "N/A")
        message = issue.get("message", "N/A")
        location = f"{component}:{line}"
        print(f"{i:<3} | {severity:<10} | {type_:<12} | {location:<40} | {message[:50]}...")

def open_issue(issue, ide_cmd="code"):
    component = issue.get("component", "").split(":")[-1]
    line = issue.get("line")
    
    # Assume workspace root is the current directory
    file_path = os.path.join(os.getcwd(), component)
    
    if not os.path.exists(file_path):
        # Try to find the file if it's not relative to root
        # This is a simple heuristic
        found = False
        for root, dirs, files in os.walk('.'):
            if component in files:
                file_path = os.path.join(root, component)
                found = True
                break
        if not found:
            print(f"Error: Could not find file {component}")
            return

    if ide_cmd == "code":
        # VS Code goto syntax: code --goto file:line
        cmd = ["code", "--goto", f"{file_path}:{line}"]
    else:
        # Generic fallback: just open the file
        cmd = [ide_cmd, file_path]
    
    print(f"Executing: {' '.join(cmd)}")
    try:
        subprocess.run(cmd, check=True, shell=False)  # NOSONAR
    except Exception as e:
        print(f"Error opening IDE: {e}")

def main():
    parser = argparse.ArgumentParser(description="SonarCloud Issue Helper CLI")
    parser.add_argument("--json", default="sonar_issues.json", help="Path to sonar_issues.json")
    parser.add_argument("--open", type=int, help="Index of the issue to open in IDE")
    parser.add_argument("--ide", default="code", help="IDE command (default: code for VS Code)")
    parser.add_argument("--list", action="store_true", help="List all issues")

    args = parser.parse_args()

    if not os.path.exists(args.json):
        print(f"Error: {args.json} not found. Run sonar_issues.py first.")
        sys.exit(1)

    with open(args.json, "r", encoding="utf-8") as f:
        issues = json.load(f)

    if args.list or args.open is None:
        list_issues(issues)
    
    if args.open is not None:
        if 0 <= args.open < len(issues):
            open_issue(issues[args.open], args.ide)
        else:
            print(f"Error: Invalid index {args.open}")

if __name__ == "__main__":
    main()
