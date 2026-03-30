#!/usr/bin/env python3
# [AXIOM ZENITH] Local Radar Pillars: On-Demand Architecture Compliance Checker
# Scans for common anti-patterns and potential security hotspots based on learned lessons.

import os
import re
import sys

# --- Configuration ---
TARGET_DIRS = ["engine", "axui", "scripts", "docker"] # Directories to scan
EXCLUDE_FILES = ["verify_zenith_pillars.py", "local_radar_pillars.py"] # Don't scan self
FILE_EXTENSIONS = ('.cpp', '.h', '.hpp', '.py', '.yml', '.md', '.conf', '.json', 'Dockerfile')

# --- Anti-Patterns (Regex based on sync_learn_from_sonar.md) ---
ANTI_PATTERNS = {
    # 1. Zero-Allocation Violations (Pillar 1)
    "P1: Dynamic Allocation": re.compile(r'\b(new |malloc\(|std::vector|std::string|std::map)\b'),
    
    # 2. Zero-Exception Violations (Pillar 5)
    "P5: Exception/RTTI": re.compile(r'\b(throw |try\s*\{|catch\s*\(|dynamic_cast|typeid\()\b'),

    # 3. Buffer Overflow & Unsafe C-Strings (Security Hotspot)
    "S: Unsafe C-String": re.compile(r'\b(strncpy|strlen\(|strcat)\b'),
    
    # 4. Cognitive Complexity (Code Smell - simplified regex)
    "C: Nested If/Loop": re.compile(r'(if|for|while)\s*\(.*\)\s*\{[^{}]*(if|for|while)\s*\(.*\)\s*\{'), # Simplified

    # 5. Security (DoS & Weak Crypto)
    "S: Weak PRNG": re.compile(r'\b(rand\()\b'), # Look for rand()
    "S: Regex DoS (Py)": re.compile(r're\.compile\(['"].*(\(.*\){2,})'), # Python regex with repetition (simplified)
    "S: Docker Root": re.compile(r'FROM\s+(ubuntu|fedora).*', re.IGNORECASE), # Check if USER is specified later

    # 6. Constructors without explicit
    "C: Missing Explicit": re.compile(r'(?!explicit\s+)(\w+::\w+)\s*\('), # Simplified

    # 7. Structural Integrity (Linkage & Capacity)
    "I: Circular Dependency Risk": re.compile(r'add_library\(.*STATIC.*\)\s+#.*(Circular|interdependent)'),
    "I: Missing Windows Console Entry": re.compile(r'add_executable\(.*\)(?!.*mainCRTStartup)'),
    "I: Capacity Mismatch (64 vs 256)": re.compile(r'FixedVector<.*,\s*64>') 
}

def scan_file(filepath):
    findings = []
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            # Special check for Dockerfiles to see if USER is specified after FROM
            if 'Dockerfile' in filepath:
                if 'FROM' in content and 'USER' not in content:
                    findings.append((0, "S: Docker Root", "Dockerfile uses root user by default (no 'USER' instruction found)."))

            for pattern_name, pattern_regex in ANTI_PATTERNS.items():
                if pattern_name == "S: Docker Root" and 'Dockerfile' not in filepath:
                    continue # Only check Dockerfiles for this pattern

                for match in pattern_regex.finditer(content):
                    line_num = content.count('
', 0, match.start()) + 1
                    findings.append((line_num, pattern_name, match.group(0).strip()))
    except Exception as e:
        print(f"Error scanning {filepath}: {e}")
    return findings

def main():
    print("---------------------------------------------------------")
    print("  [AXIOM] LOCAL RADAR PILLARS: ON-DEMAND COMPLIANCE SCAN ")
    print("---------------------------------------------------------")
    
    total_issues = 0
    all_findings = []

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, os.pardir))

    for target_dir_name in TARGET_DIRS:
        target_dir_path = os.path.join(project_root, target_dir_name)
        if not os.path.isdir(target_dir_path):
            continue

        for root, _, files in os.walk(target_dir_path):
            for file in files:
                if not file.endswith(FILE_EXTENSIONS) or file in EXCLUDE_FILES:
                    continue
                
                filepath = os.path.join(root, file)
                findings = scan_file(filepath)
                if findings:
                    all_findings.append((filepath, findings))
                    total_issues += len(findings)

    if total_issues > 0:
        print(f"
[CRITICAL] Found {total_issues} potential compliance issues:")
        for filepath, findings in all_findings:
            print(f"
--- {os.path.relpath(filepath, project_root)} ---")
            for line, pattern_name, matched_text in findings:
                print(f"  Line {line}: [{pattern_name}] -> '{matched_text}'")
        
        print("
Refer to .agents/sync_learn_from_sonar.md for remediation guidelines.")
        # sys.exit(1) # Optionally, exit with error for CI integration
    else:
        print("
[SUCCESS] No common anti-patterns found in target directories.")

if __name__ == "__main__":
    main()
