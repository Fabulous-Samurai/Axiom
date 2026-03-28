import subprocess
import sys

def run_build():
    print("Starting build process...")
    process = subprocess.Popen(
        ["cmake", "--build", "build-test", "--target", "axiom_engine_core", "-j", "4"],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1
    )
    
    for line in iter(process.stdout.readline, ''):
        print(line, end='', flush=True)
        
    process.wait()
    print(f"Build finished with exit code {process.returncode}")
    sys.exit(process.returncode)

if __name__ == "__main__":
    run_build()