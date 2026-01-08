#!/usr/bin/env python
"""Test modulus operation."""
from gui.python.gui_helpers import CppEngineInterface

engine = CppEngineInterface('ninja-build/axiom.exe')
print("Testing modulus operation...")

# Test with spaces
result = engine.execute_command('938247290474946 % 342423423')
print(f"Result with spaces: {result}")

# Test without spaces  
result = engine.execute_command('938247290474946%342423423')
print(f"Result without spaces: {result}")

engine.close()
