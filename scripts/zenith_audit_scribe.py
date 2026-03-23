import subprocess
import json
import os
import time
import shlex
import sys

def run_cmd(cmd):
    try:
        if isinstance(cmd, str):
            cmd = shlex.split(cmd, posix=sys.platform != "win32")
        result = subprocess.run(cmd, shell=False, capture_output=True, text=True, timeout=300)
        return {
            "cmd": cmd,
            "success": result.returncode == 0,
            "stdout": result.stdout,
            "stderr": result.stderr,
            "returncode": result.returncode
        }
    except Exception as e:
        return {
            "cmd": cmd,
            "success": False,
            "error": str(e)
        }

def main():
    report = {
        "timestamp": time.ctime(),
        "audit_results": []
    }

    checks = [
        # TLA+ (Using relative path to jar)
        "java -jar tools/tla/tla2tools.jar -config formal/tla/MantisAStarCorrectness.cfg formal/tla/MantisAStarCorrectness.tla",
        "java -jar tools/tla/tla2tools.jar -config formal/tla/MantisHeuristicDispatch.cfg formal/tla/MantisHeuristicDispatch.tla",
        "java -jar tools/tla/tla2tools.jar -config formal/tla/HarmonicArenaSafety.cfg formal/tla/HarmonicArenaSafety.tla",
        "java -jar tools/tla/tla2tools.jar -config formal/tla/MantisSecureVaultSafety.cfg formal/tla/MantisSecureVaultSafety.tla",
        
        # Pillars
        "python scripts/verify_zenith_pillars.py",
        
        # C++ Tests (use / for cross-platform compatibility if possible, but keeping \ for Windows compatibility)
        "build\\run_tests.exe" if sys.platform == "win32" else "./build/run_tests",

        # Python Packaging
        "python -m build --wheel"
    ]

    for check in checks:
        print(f"Executing: {check}")
        res = run_cmd(check)
        report["audit_results"].append(res)

    with open("zenith_readiness_report.json", "w") as f:
        json.dump(report, f, indent=4)
    
    print("Audit Complete. Report saved to zenith_readiness_report.json")

if __name__ == "__main__":
    main()
