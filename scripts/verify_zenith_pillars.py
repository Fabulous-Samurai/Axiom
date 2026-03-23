#!/usr/bin/env python3
"""
[AXIOM ZENITH PILLAR VERIFIER]
Enforces Pillar 1 (Zero-Allocation) and Pillar 5 (Zero-Exception)
by scanning for forbidden Keywords in the hot-path sources.
"""

import os

# Forbidden patterns for Zenith compliance
FORBIDDEN_KEYWORDS = {
    # Pillar 1 & 3 violations
    "malloc(": "Pillar 1: Dynamic allocation (malloc) detected outside allowed pools.",
    "free(": "Pillar 1: Manual deallocation (free) detected.",
    "new ": "Pillar 1: Heap allocation (new) detected.",
    "delete ": "Pillar 1: Heap deallocation (delete) detected.",
    "std::vector": "Pillar 1/3: std::vector detected. Use AXIOM::FixedVector or Arena instead.",
    
    # Pillar 5 violations
    "throw ": "Pillar 5: Exception throwing detected. Zenith Core must be Zero-Exception.",
    "try {": "Pillar 5: Exception handling (try) detected.",
    "catch (": "Pillar 5: Exception handling (catch) detected.",
}

# Exempt files (main, setup, etc.)
EXEMPT_FILES = ["main.cpp", "setup", "test", "nanobind"]

def verify_file(file_path):
    violations = []
    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
        for i, line in enumerate(lines):
            # Skip comments
            if line.strip().startswith("//") or line.strip().startswith("/*"):
                continue
            
            for keyword, message in FORBIDDEN_KEYWORDS.items():
                if keyword in line:
                    violations.append(f"Line {i+1}: {message}")
    return violations

def main():
    print("--------------------------------------------------------")
    print("  [AXIOM] ZENITH PILLAR VERIFIER: SCANNIG CORE...      ")
    print("--------------------------------------------------------")
    
    core_dirs = ["src", "include", "core"]
    total_violations = 0
    
    for d in core_dirs:
        if not os.path.exists(d): continue
        for root, _, files in os.walk(d):
            for file in files:
                if any(exempt in file for exempt in EXEMPT_FILES):
                    continue
                if file.endswith((".cpp", ".h", ".hpp", ".cc")):
                    path = os.path.join(root, file)
                    violations = verify_file(path)
                    if violations:
                        print(f"[FAIL] {path}")
                        for v in violations:
                            print(f"  - {v}")
                        total_violations += len(violations)
                    else:
                        print(f"[PASS] {path}")
                        
    if total_violations > 0:
        print(f"\n[CRITICAL] Found {total_violations} Zenith Pillar violations.")
        # sys.exit(1) # Un-comment to fail CI on violations
    else:
        print("\n[SUCCESS] All core modules comply with Zenith Pillars.")
        
if __name__ == "__main__":
    main()
