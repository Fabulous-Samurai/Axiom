#!/usr/bin/env python3
"""
🏎️ SENNA SPEED TEST - Monaco GP Performance! 🏎️
Test the ultra-fast C++ engine performance
"""

import subprocess
import time
import sys
from pathlib import Path

def senna_speed_test():
    """Test C++ engine for Senna-level performance"""
    print("🏎️ SENNA SPEED TEST - Monaco GP Performance! 🏎️")
    print("=" * 50)
    
    # Find the C++ executable
    current_dir = Path(__file__).parent.parent.parent  # Go back to project root
    executable_path = None
    possible_paths = [
        current_dir / "ninja-build" / "axiom.exe",
        current_dir / "build" / "axiom.exe",
        current_dir / "build-ninja" / "cpp_dynamic_calc.exe",
        current_dir / "build" / "cpp_dynamic_calc.exe",
        current_dir / "build-ninja" / "Debug" / "cpp_dynamic_calc.exe",
    ]
    
    for path in possible_paths:
        if path.exists():
            executable_path = str(path)
            print(f"✅ Found AXIOM executable: {executable_path}")
            break
    
    if not executable_path:
        print("❌ C++ executable not found!")
        return
    
    print(f"🚀 Found C++ engine: {executable_path}")
    
    # Test simple arithmetic operations - should be LIGHTNING FAST! ⚡
    test_cases = [
        "12+21",
        "2*3", 
        "15/3",
        "10-5",
        "2^8",
        "sqrt(16)",
        "sin(0)",
        "100*100"
    ]
    
    total_time = 0
    successful_tests = 0
    
    print("\n🏁 Starting Senna Speed Tests...")
    print("Target: Under 100ms per operation (Senna speed!)")
    print("-" * 50)
    
    for i, expression in enumerate(test_cases, 1):
        print(f"🏎️ Test {i}/8: {expression:<10}", end=" → ")
        
        start_time = time.perf_counter()
        
        try:
            # 🏎️ SENNA SPEED - Use command-line arguments for ultra-fast execution! 
            result = subprocess.run(
                [executable_path, expression],  # Pass expression as argument!
                capture_output=True,
                text=True,
                timeout=0.5,  # Much shorter timeout - should be INSTANT!
                encoding='utf-8',
                errors='ignore',
                creationflags=subprocess.CREATE_NO_WINDOW if sys.platform == 'win32' else 0
            )
            
            end_time = time.perf_counter()
            execution_time = (end_time - start_time) * 1000  # Convert to ms
            total_time += execution_time
            
            if result.returncode == 0:
                # Parse the direct output - should be just the number!
                result_text = result.stdout.strip()
                
                if result_text:
                    # Performance classification
                    if execution_time < 50:
                        performance = "🏎️ SENNA SPEED!"
                    elif execution_time < 100:
                        performance = "🚀 F1 SPEED"
                    elif execution_time < 200:
                        performance = "🏁 Racing"
                    else:
                        performance = "🐌 Slow"
                    
                    print(f"{result_text:<8} ({execution_time:.1f}ms) {performance}")
                    successful_tests += 1
                else:
                    print(f"No result ({execution_time:.1f}ms) ❌")
            else:
                error_msg = result.stderr.strip() if result.stderr else "Unknown error"
                print(f"Error: {error_msg} ({execution_time:.1f}ms) ❌")
                
        except subprocess.TimeoutExpired:
            end_time = time.perf_counter()
            execution_time = (end_time - start_time) * 1000
            print(f"TIMEOUT ({execution_time:.1f}ms) 💥")
            total_time += execution_time
            
        except Exception as e:
            end_time = time.perf_counter()
            execution_time = (end_time - start_time) * 1000
            print(f"Exception ({execution_time:.1f}ms) 💥")
            total_time += execution_time
    
    # Performance summary
    print("\n" + "=" * 50)
    print("🏁 SENNA SPEED TEST RESULTS")
    print("-" * 50)
    
    avg_time = total_time / len(test_cases)
    success_rate = (successful_tests / len(test_cases)) * 100
    
    print(f"✅ Successful operations: {successful_tests}/{len(test_cases)} ({success_rate:.1f}%)")
    print(f"⚡ Average execution time: {avg_time:.1f}ms")
    print(f"🎯 Total test time: {total_time:.1f}ms")
    
    # Performance rating
    if avg_time < 50 and success_rate > 80:
        print("🏆 RESULT: AYRTON SENNA LEVEL PERFORMANCE! 🏎️🏎️🏎️")
        print("   Monaco GP speed achieved! Lightning fast!")
    elif avg_time < 100 and success_rate > 70:
        print("🥇 RESULT: F1 CHAMPION LEVEL! 🏁")
        print("   Excellent performance, race-worthy speed!")
    elif avg_time < 200 and success_rate > 50:
        print("🥈 RESULT: Racing Level Performance 🏁")
        print("   Good speed, could use some optimization")
    else:
        print("🐌 RESULT: Needs turbo boost!")
        print("   Time for engine optimization!")
    
    # Recommendations
    if avg_time > 100:
        print("\n🔧 OPTIMIZATION TIPS:")
        print("   • Use persistent C++ process with pipes")
        print("   • Minimize subprocess creation overhead")  
        print("   • Implement direct memory communication")
        print("   • Consider C++ shared library (DLL/SO)")

if __name__ == "__main__":
    senna_speed_test()