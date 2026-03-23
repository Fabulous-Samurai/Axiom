import shutil
import os

files_to_move = {
    'refactor_to_gtest.py': 'scripts/',
    'sonar_coverage_analysis.py': 'scripts/',
    'run_tests_safely.py': 'scripts/',
    'sonar_issues.json': 'reports/sonar/',
    'sonar_tasks.md': 'reports/sonar/',
    'benchmark_results.csv': 'reports/benchmarks/',
    'benchmark_results.json': 'reports/benchmarks/',
    'benchmark_results.md': 'reports/benchmarks/',
    'config.json': 'config/',
    'requirements-optional.txt': 'requirements/'
}

for src, dst in files_to_move.items():
    if os.path.exists(src):
        try:
            print(f"Moving {src} to {dst}")
            shutil.move(src, os.path.join(dst, src))
        except Exception as e:
            print(f"Failed to move {src}: {e}")
    else:
        print(f"File not found: {src}")
