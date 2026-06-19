import subprocess
import sys
import os
from datetime import datetime

def run_check(config_name):
    jar_path = "tla2tools.jar"
    tla_file = "PlutoPetriNet.tla"
    cfg_file = f"PlutoPetriNet_{config_name}.cfg"
    log_file = os.path.join("logs", f"verification_{config_name}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log")

    cmd = ["java", "-cp", jar_path, "tlc2.TLC", "-config", cfg_file, tla_file]

    print(f"Running verification: {config_name}...")
    with open(log_file, "w") as f:
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        f.write(result.stdout)

    if result.returncode == 0:
        print(f"[SUCCESS] {config_name} passed. Log: {log_file}")
    else:
        print(f"[FAIL] {config_name} failed. Check log: {log_file}")

if __name__ == '__main__':
    configs = ["Quick", "Standard"]
    if len(sys.argv) > 1 and sys.argv[1] == "--full":
        configs.append("Full")

    for cfg in configs:
        run_check(cfg)
