import subprocess
import json
import os
import time

def run_cmd(cmd):
    cmd_str = " ".join(cmd)
    try:
        result = subprocess.run(cmd, shell=False, capture_output=True, text=True, timeout=300)  # NOSONAR
        return {
            "cmd": cmd_str,
            "success": result.returncode == 0,
            "stdout": result.stdout,
            "stderr": result.stderr,
            "returncode": result.returncode
        }
    except Exception as e:
        return {
            "cmd": cmd_str,
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
        ["java", "-jar", os.path.join("tools", "tla", "tla2tools.jar"), "-config", os.path.join("formal", "tla", "MantisAStarCorrectness.cfg"), os.path.join("formal", "tla", "MantisAStarCorrectness.tla")],
        ["java", "-jar", os.path.join("tools", "tla", "tla2tools.jar"), "-config", os.path.join("formal", "tla", "MantisHeuristicDispatch.cfg"), os.path.join("formal", "tla", "MantisHeuristicDispatch.tla")],
        ["java", "-jar", os.path.join("tools", "tla", "tla2tools.jar"), "-config", os.path.join("formal", "tla", "HarmonicArenaSafety.cfg"), os.path.join("formal", "tla", "HarmonicArenaSafety.tla")],
        ["java", "-jar", os.path.join("tools", "tla", "tla2tools.jar"), "-config", os.path.join("formal", "tla", "MantisSecureVaultSafety.cfg"), os.path.join("formal", "tla", "MantisSecureVaultSafety.tla")],
        
        # Pillars
        ["python", os.path.join("scripts", "verify_zenith_pillars.py")],
        
        # C++ Tests
        [os.path.join("build", "run_tests.exe")],
        
        # Python Packaging
        ["python", "-m", "build", "--wheel"]
    ]

    for check in checks:
        print(f"Executing: {' '.join(check)}")
        res = run_cmd(check)
        report["audit_results"].append(res)

    with open("zenith_readiness_report.json", "w") as f:
        json.dump(report, f, indent=4)
    
    print("Audit Complete. Report saved to zenith_readiness_report.json")

if __name__ == "__main__":
    main()
