#!/usr/bin/env python3
"""Simple Windows-friendly repository scanner for forbidden allocation/exception patterns.
Writes JSON summary to --out and a human-readable text file to --text.
"""
import os
import re
import json
import argparse

PATTERNS = [
    r"\bstd::vector\b",
    r"\bstd::string\b",
    r"\bnew\b",
    r"\bmalloc\(" ,
    r"\bfree\(" ,
    r"\bthrow\b",
    r"\bstd::mutex\b",
    r"\bstd::unique_ptr\b",
    r"\bstd::shared_ptr\b",
]

def scan(root):
    results = []
    for dirpath, dirnames, filenames in os.walk(root):
        if should_skip_dir(dirpath):
            continue
        for fn in filenames:
            if not is_valid_file(fn):
                continue
            path = os.path.join(dirpath, fn)
            results.extend(scan_file(path))
    return results

def should_skip_dir(dirpath):
    # Skip build, node_modules, .git, and external dependencies
    skip_names = {'build', 'node_modules', '.git', '_deps', 'extern', 'mimalloc-src', 'ninja-build'}
    parts = set(re.split(r'[\\/]', dirpath))
    if any(name in parts for name in skip_names):
        return True
    return False

def is_valid_file(filename):
    # Only scan core source files for zero-allocation/no-exception compliance
    return filename.endswith(('.cpp', '.h', '.hpp'))

def scan_file(path):
    results = []
    try:
        with open(path, 'r', encoding='utf-8', errors='ignore') as f:
            for i, line in enumerate(f, start=1):
                results.extend(check_patterns(path, line, i))
    except Exception:
        pass
    return results

def check_patterns(path, line, line_number):
    matches = []
    for pat in PATTERNS:
        if re.search(pat, line):
            matches.append({'file': path, 'line': line_number, 'pattern': pat, 'text': line.rstrip('\n')})
    return matches

def write_outputs(results, out_json, out_text):
    os.makedirs(os.path.dirname(out_json), exist_ok=True)
    os.makedirs(os.path.dirname(out_text), exist_ok=True)
    with open(out_json, 'w', encoding='utf-8') as j:
        json.dump(results, j, indent=2)

    grouped = {}
    for r in results:
        grouped.setdefault(r['file'], []).append(r)

    with open(out_text, 'w', encoding='utf-8') as t:
        for f, items in grouped.items():
            t.write(f"--- File: {f} ---\n")
            t.write(f"Total issues: {len(items)}\n")
            for it in items:
                t.write(f"[WARN] Line {it['line']} ({it['pattern']}): {it['text']}\n")
            t.write('\n')

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--out', default='reports/zero_alloc_scan_summary.json')
    p.add_argument('--text', default='output/parsed_issues.txt')
    args = p.parse_args()
    results = scan(os.getcwd())
    write_outputs(results, args.out, args.text)
    print(f"Scan complete: {len(results)} hits. Wrote {args.out} and {args.text}")

if __name__ == '__main__':
    main()
