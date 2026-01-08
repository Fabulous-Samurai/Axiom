#!/usr/bin/env python3
"""Test axiom.exe execution"""
import sys
sys.path.insert(0, "gui/python")

from gui_helpers import CppEngineInterface

# Test the engine
engine = CppEngineInterface("ninja-build/axiom.exe")

tests = [
    "2+2",
    "5*3",
    "10/2",
    "sqrt(16)",
    "sin(0)",
]

print("Testing axiom.exe with single-command mode:")
print("=" * 50)

for test in tests:
    result = engine.execute_command(test)
    if result["success"]:
        print(f"✓ {test} = {result['result']} ({result['execution_time']}ms)")
    else:
        print(f"✗ {test} failed: {result.get('error', 'Unknown error')}")

print("\nEngine test complete!")
