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

import subprocess

def get_modified_files():
    try:
        # Get list of tracked modified files
        result = subprocess.run(
            ['git', 'diff', '--name-only', '--cached'],
            capture_output=True, text=True, check=True, shell=False
        )
        staged_files = set(result.stdout.strip().split('\n'))

        result2 = subprocess.run(
            ['git', 'diff', '--name-only'],
            capture_output=True, text=True, check=True, shell=False
        )
        unstaged_files = set(result2.stdout.strip().split('\n'))

        # Get untracked files
        result3 = subprocess.run(
            ['git', 'ls-files', '--others', '--exclude-standard'],
            capture_output=True, text=True, check=True, shell=False
        )
        untracked_files = set(result3.stdout.strip().split('\n'))

        all_modified = staged_files | unstaged_files | untracked_files
        # Filter out empty strings
        return {f for f in all_modified if f}
    except Exception as e:
        print(f"[WARNING] Failed to determine modified files via git: {e}")
        # In CI, if this fails, we might fall back to checking all or nothing, but usually we
        # want to check files modified between PR source and target branches.
        # For simplicity and robustness in this CI context, we can just check git diff head
        try:
            result = subprocess.run(
                ['git', 'diff', '--name-only', 'HEAD'],
                capture_output=True, text=True, check=True, shell=False
            )
            return {f for f in result.stdout.strip().split('\n') if f}
        except Exception:
            pass

        # For GitHub Actions pull requests, we can use the default branch as reference
        try:
            # This relies on actions/checkout having fetched the origin
            result = subprocess.run(
                ['git', 'diff', '--name-only', 'origin/main...HEAD'],
                capture_output=True, text=True, check=True, shell=False
            )
            files = {f for f in result.stdout.strip().split('\n') if f}
            if files:
                return files
        except Exception:
            pass

        # If running in GitHub Actions PR, we can use env vars to find changed files,
        # but as a fallback, return None to indicate "check all"
        return None

def main():
    print("--------------------------------------------------------")
    print("  [AXIOM] ZENITH PILLAR VERIFIER: MANDATORY AUDIT      ")
    print("--------------------------------------------------------")
    
    # Target core directories
    core_dirs = ["engine/core", "engine/compute", "engine/ipc", "engine/api"]
    total_violations = 0
    
    modified_files = get_modified_files()
    if modified_files is not None:
        print(f"Checking only modified files: {len(modified_files)} files found.")
    else:
        print("Checking all files in target directories.")

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
                    # Normalize path for matching with git diff output
                    # Remove relative prefix if script was run from inside 'scripts/'
                    norm_path = os.path.normpath(path).replace('\\', '/')
                    if norm_path.startswith('../'):
                        norm_path = norm_path[3:]

                    if modified_files is not None and norm_path not in modified_files:
                        continue

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
