import sys
import os
import subprocess

# Add scripts directory to path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../scripts')))

try:
    from sandbox import run_isolated_expression
except ImportError:
    print("Error: sandbox.py not found.")
    sys.exit(1)

def test_file_system_escape():
    print("[INFO] Attempting File System Escape...")
    # Attempt to read a sensitive file (simulated)
    res = run_isolated_expression("__import__('os').listdir('C:/')")
    print(f"Result: {res}")

def test_infinite_memory():
    print("[INFO] Attempting Memory Exhaustion...")
    # Attempt to allocate a huge list
    res = run_isolated_expression("[0] * (10**9)")
    print(f"Result: {res}")

def test_infinite_loop():
    print("[INFO] Attempting Infinite Loop (Timeout)...")
    res = run_isolated_expression("9**9999999")
    print(f"Result: {res}")

if __name__ == "__main__":
    test_file_system_escape()
    test_infinite_memory()
    test_infinite_loop()
    print("\n[INFO] Sandbox escape attempts completed.")
