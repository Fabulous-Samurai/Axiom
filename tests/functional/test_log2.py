import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent / "gui/python"))
from gui_helpers import CppEngineInterface

engine = CppEngineInterface("ninja-build/axiom.exe")

print("Testing log2 function...")
result = engine.execute_command("log2(8)")
print(f"log2(8) = {result}")

result2 = engine.execute_command("log2(16)")
print(f"log2(16) = {result2}")

result3 = engine.execute_command("log2(1024)")
print(f"log2(1024) = {result3}")

engine.close()
