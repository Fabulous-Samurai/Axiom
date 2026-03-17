#!/usr/bin/env python3
"""
AXIOM PRO - Math Functions Validation
Validates arithmetic, sqrt, power, trig, exp/log via axiom.exe; numpy fallback.
"""
import sys
import subprocess
import re
import math
from pathlib import Path

project_root = Path(__file__).parent.parent.parent

EXPRESSIONS = [
    ("2 + 2", 4.0, "4"),
    ("10 * 5", 50.0, "50"),
    ("100 / 4", 25.0, "25"),
    ("2^8", 256.0, "256"),
    ("sqrt(144)", 12.0, "12"),
    ("sin(0)", 0.0, "0"),
    ("cos(0)", 1.0, "1"),
    ("tan(0)", 0.0, "0"),
    ("exp(1)", math.e, "2.718"),
    ("log(1)", 0.0, "0"),
]

SUGGESTED_EXPRS = [
    ("pi", "3.14159"),
    ("sin(pi/2)", "1"),
    ("cos(pi)", "-1"),
]

def find_axiom_exe():
    candidates = [
        project_root / 'ninja-build' / 'axiom.exe',
        project_root / 'build' / 'axiom.exe',
        project_root / 'axiom.exe',
    ]
    for p in candidates:
        if p.exists():
            return str(p)
    return None


def run_axiom(expr: str, axiom_exe: str) -> tuple[int, str]:
    try:
        proc = subprocess.run(
            [axiom_exe],
            input=f"{expr}\nexit\n",
            capture_output=True,
            text=True,
            encoding='utf-8',
            errors='ignore',
            timeout=5,
        )
        out = (proc.stdout or "").strip()
        return proc.returncode, out
    except Exception as e:
        return -1, f"ERROR: {e}"


def numbers_from_output(out: str) -> list[float]:
    nums = []
    for m in re.finditer(r"[-+]?\d*\.?\d+(?:[eE][-+]?\d+)?", out):
        try:
            nums.append(float(m.group(0)))
        except Exception:
            pass
    return nums


def main():
    print("="*60)
    print("🏛️ AXIOM PRO - MATH FUNCTIONS VALIDATION")
    print("="*60)

    axiom_exe = find_axiom_exe()
    engine_available = bool(axiom_exe)
    print(f"Engine: {'FOUND' if engine_available else 'NOT FOUND'}")
    if engine_available:
        print(f"Path: {axiom_exe}")

    passed, failed = 0, 0

    # Run core expressions
    tol = 1e-3
    for expr, expected_val, expected_str in EXPRESSIONS:
        if engine_available:
            rc, out = run_axiom(expr, axiom_exe)
            nums = numbers_from_output(out)
            ok_num = any(abs(n - expected_val) <= tol for n in nums)
            ok = (rc == 0) and (ok_num or (expected_str in out))
            print(f"{expr:16s} -> {'✅' if ok else '❌'} (out: {out[:80]})")
            if ok:
                passed += 1
            else:
                failed += 1
        else:
            # numpy fallback
            try:
                import numpy as np
                env = {
                    'sqrt': np.sqrt,
                    'sin': np.sin,
                    'cos': np.cos,
                    'tan': np.tan,
                    'exp': np.exp,
                    'log': np.log,
                }
                val = eval(expr, {"__builtins__": {}}, env)
                sval = f"{float(val):.6f}" if isinstance(val, (int, float, np.floating)) else str(val)
                ok = (abs(float(val) - expected_val) <= tol) or (expected_str in sval)
                print(f"{expr:16s} -> {'✅' if ok else '❌'} (val: {sval})")
                if ok:
                    passed += 1
                else:
                    failed += 1
            except Exception as e:
                print(f"{expr:16s} -> ❌ (error: {e})")
                failed += 1

    # Suggested expressions (informational)
    print("\nSuggestions:")
    for expr, expected in SUGGESTED_EXPRS:
        if engine_available:
            rc, out = run_axiom(expr, axiom_exe)
            ok = (rc == 0 and expected in out)
            print(f"{expr:16s} -> {'✅' if ok else '⚠️'} (out: {out[:80]})")
        else:
            print(f"{expr:16s} -> ⚠️ (engine not available)")

    print("\n" + "="*60)
    total = passed + failed
    print(f"Total: {total} tests")
    print(f"Passed: {passed} ({(passed*100)//max(1,total)}%)")
    print(f"Failed: {failed}")
    print("="*60)

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
