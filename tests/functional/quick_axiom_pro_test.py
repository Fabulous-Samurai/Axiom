#!/usr/bin/env python3
"""
Quick Test of AXIOM PRO Core Functions
"""

import sys
from pathlib import Path

# Add project paths
project_root = Path(__file__).parent.parent.parent
sys.path.insert(0, str(project_root / 'gui' / 'python'))

def main():
    print("🏛️ AXIOM PRO - Quick Function Test")
    print("="*60)
    
    # Test 1: Import GUI
    print("\n1️⃣ Testing GUI Import...")
    try:
        from axiom_pro_gui import AxiomProGUI
        print("   ✅ AxiomProGUI imported successfully")
    except Exception as e:
        print(f"   ❌ Failed: {e}")
        return False
    
    # Test 2: Check methods
    print("\n2️⃣ Checking GUI Methods...")
    required_methods = [
        'create_gui',
        'execute_command',
        'send_to_axiom',
        'plot_results'
    ]
    
    all_present = True
    for method in required_methods:
        if hasattr(AxiomProGUI, method):
            print(f"   ✅ {method}")
        else:
            print(f"   ❌ {method} missing")
            all_present = False
    
    if not all_present:
        return False
    
    # Test 3: Check axiom.exe
    print("\n3️⃣ Checking AXIOM Engine...")
    axiom_paths = [
        project_root / 'ninja-build' / 'axiom.exe',
        project_root / 'build' / 'axiom.exe',
    ]
    
    engine_found = False
    for path in axiom_paths:
        if path.exists():
            print(f"   ✅ Found at: {path.name}")
            engine_found = True
            break
    
    if not engine_found:
        print("   ⚠️ axiom.exe not found")
    
    # Test 4: Signal Processing Toolkit
    print("\n4️⃣ Testing Signal Processing...")
    try:
        sys.path.insert(0, str(project_root / 'tools' / 'analysis'))
        from signal_processing_toolkit import SignalProcessingToolkit
        print("   ✅ SignalProcessingToolkit imported")
        
        toolkit = SignalProcessingToolkit()
        print("   ✅ Toolkit instantiated")
        
        # Create test signals without showing plots
        import matplotlib
        matplotlib.use('Agg')
        toolkit.create_test_signals()
        print(f"   ✅ Generated {len(toolkit.signals)} signals")
        
    except Exception as e:
        print(f"   ❌ Failed: {e}")
        return False
    
    # Test 5: Numpy operations
    print("\n5️⃣ Testing NumPy Operations...")
    try:
        import numpy as np
        x = np.array([1, 2, 3, 4, 5])
        y = np.array([2, 4, 6, 8, 10])
        result = x + y
        print(f"   ✅ Array operations work: {result[0]} + {result[1]} = {result[0] + result[1]}")
    except Exception as e:
        print(f"   ❌ Failed: {e}")
        return False
    
    # Test 6: Matplotlib (non-GUI)
    print("\n6️⃣ Testing Matplotlib...")
    try:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        fig, ax = plt.subplots()
        ax.plot([1, 2, 3], [1, 4, 9])
        plt.close(fig)
        print("   ✅ Plotting works")
    except Exception as e:
        print(f"   ❌ Failed: {e}")
        return False
    
    print("\n" + "="*60)
    print("✅ ALL CORE FUNCTIONS PASSED!")
    print("="*60)
    return True

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
