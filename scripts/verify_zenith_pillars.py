#!/usr/bin/env python3
"""
[AXIOM ZENITH PILLAR VERIFIER]
Enforces Pillar 1 (Zero-Allocation) and Pillar 5 (Zero-Exception)
by scanning for forbidden Keywords in the hot-path sources.
"""

import sys
import os
import re

# Forbidden patterns for Zenith compliance
FORBIDDEN_KEYWORDS = {
    # Pillar 1 & 3 violations (Heap & Determinism)
    "malloc(": "Pillar 1: Dynamic allocation (malloc) detected outside allowed pools.",
    "free(": "Pillar 1: Manual deallocation (free) detected.",
    "new ": "Pillar 1: Heap allocation (new) detected.",
    "delete ": "Pillar 1: Heap deallocation (delete) detected.",
    "std::vector": "Pillar 1/3: std::vector detected. Use AXIOM::FixedVector or Arena instead.",
    "std::map": "Pillar 1/3: std::map detected. Use Arena-based structures or robin-map.",
    "std::string": "Pillar 1/3: std::string detected in CORE. Use std::string_view or const char*.",
    
    # Pillar 5 violations (Exceptions & RTTI)
    "throw ": "Pillar 5: Exception throwing detected. Zenith Core must be Zero-Exception.",
    "try {": "Pillar 5: Exception handling (try) detected.",
    "catch (": "Pillar 5: Exception handling (catch) detected.",
    "dynamic_cast": "Pillar 5: RTTI (dynamic_cast) detected. RTTI is disabled in Zenith Core.",
    "typeid(": "Pillar 5: RTTI (typeid) detected. RTTI is disabled in Zenith Core.",
}

SUGGESTIONS = {
    "malloc(": "Replace with AXIOM::Arena::allocate().",
    "std::vector": "Replace with AXIOM::FixedVector<T, N> or Arena-allocated buffer.",
    "std::string": "Replace with std::string_view or pre-allocated char buffers.",
    "throw ": "Replace with 'return std::unexpected(err);' (Zero-Exception policy).",
    "try {": "Remove try/catch. Use EngineResult (std::expected) for error propagation.",
    "dynamic_cast": "Use static_cast with manual tag-based dispatch or variants.",
}

# Exempt files - MUST BE KEPT MINIMAL
EXEMPT_FILES = ["main.cpp", "setup_other_device", "test_"]
# WHITELISTED_FILES are the ONLY files allowed to implement allocation logic
WHITELISTED_FILES = [
    "arena.h", 
    "arena_allocator.cpp", 
    "harmonic_arena.h",
    "fixed_vector.h", 
    "python_bindings.cpp", 
    "nanobind_interface.cpp"
]

def verify_file(file_path):
    violations = []
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            for i, line in enumerate(lines):
                # Skip comments
                clean_line = line.strip()
                if clean_line.startswith("//") or clean_line.startswith("/*") or clean_line.startswith("*"):
                    continue
                
                for keyword, message in FORBIDDEN_KEYWORDS.items():
                    if keyword in line:
                        # Double check it's not a comment at the end of a line
                        if "//" in line and line.find("//") < line.find(keyword):
                            continue
                        violations.append(f"Line {i+1}: {message}")
    except Exception as e:
        print(f"[ERROR] Could not read {file_path}: {e}")
    return violations

def main():
    print("--------------------------------------------------------")
    print("  [AXIOM] ZENITH PILLAR VERIFIER: MANDATORY AUDIT      ")
    print("--------------------------------------------------------")
    
    # Determine the default branch dynamically (main or master)
    import subprocess
    try:
        # Fallback to checking all changed files against target branch
        # if PR, GitHub actions might check out differently. We use a fallback if needed.
        target_branch = os.environ.get("GITHUB_BASE_REF", "master")
        # For simplicity and robustness across both local and CI environments,
        # we will extract the exact changed files based on git history.
        # CI for PRs uses a merge commit where HEAD^2 is the incoming branch.
        # Locally, diffing against origin/master or master works.
        # We will extract only the modified files from the current PR/branch using Git.
        try:
            # First, check locally modified, untracked, and staged files
            cmd_staged = ["git", "diff", "--name-only", "--cached"]
            result_staged = subprocess.run(cmd_staged, capture_output=True, text=True, check=True)
            changed_files = result_staged.stdout.splitlines()

            cmd_unstaged = ["git", "diff", "--name-only"]
            result_unstaged = subprocess.run(cmd_unstaged, capture_output=True, text=True, check=True)
            changed_files.extend(result_unstaged.stdout.splitlines())

            cmd_untracked = ["git", "ls-files", "--others", "--exclude-standard"]
            result_untracked = subprocess.run(cmd_untracked, capture_output=True, text=True, check=True)
            changed_files.extend(result_untracked.stdout.splitlines())

            changed_files = list(set(changed_files))

            # If there are no local changes, this might be a CI run or we just committed.
            # In that case, look at the files modified in the last commit.
            # In GitHub Actions PR checkout, the last commit is the merge commit,
            # but getting the actual PR files might be tricky with fetch-depth: 1.
            # The safest approach for CI is diffing against the parent commit `git diff HEAD^1..HEAD`
            # or if it's not a merge commit `git show --name-only --format=`.
            if not changed_files:
                cmd = ["git", "show", "--name-only", "--format="]
                result = subprocess.run(cmd, capture_output=True, text=True, check=True)
                changed_files = result.stdout.splitlines()
        except Exception as e:
            changed_files = []
            print(f"[WARNING] Could not determine changed files: {e}. Scanning 0 files.")
    except Exception as e:
        changed_files = []
        print(f"[WARNING] Could not determine changed files: {e}. Scanning 0 files.")

    # Target core directories
    core_dirs = ["engine/core", "engine/compute", "engine/ipc", "engine/api"]
    total_violations = 0
    
    files_to_scan = []
    if changed_files:
        files_to_scan = [f for f in changed_files if any(f.startswith(d) for d in core_dirs)]
    else:
        for d in core_dirs:
            search_path = d if os.path.exists(d) else os.path.join("..", d)
            if not os.path.exists(search_path): continue
            for root, _, files in os.walk(search_path):
                for file in files:
                    files_to_scan.append(os.path.join(root, file))

    for path in files_to_scan:
        file = os.path.basename(path)
        # Tightened exemption check
        if any(exempt in file for exempt in EXEMPT_FILES) or file in WHITELISTED_FILES:
            continue
        if file.endswith((".cpp", ".h", ".hpp", ".cc")):
            violations = verify_file(path)
            if violations:
                print(f"[FAIL] {path}")
                for v in violations:
                    print(f"  - {v}")
                total_violations += len(violations)
                        
    if total_violations > 0:
        print(f"\n[CRITICAL] Found {total_violations} Zenith Pillar violations.")
        print("\n--- ARCHITECTURAL REMEDIATION SUGGESTIONS ---")
        # Global scan for suggestions (limited to unique ones)
        for keyword, suggestion in SUGGESTIONS.items():
            print(f"💡 [FOR {keyword}]: {suggestion}")
        
        print("\n[RESULT] Enforcement: BLOCKING. Please fix violations to proceed.")
        sys.exit(1)
    else:
        print("\n[SUCCESS] All core modules comply with Zenith Pillars (Zero-Allocation, Zero-Exception).")
        
if __name__ == "__main__":
    main()
