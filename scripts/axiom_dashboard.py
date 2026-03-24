#!/usr/bin/env python3
import argparse
import os
import sys
import subprocess
import shutil
from pathlib import Path

def main():
    parser = argparse.ArgumentParser(description="AXIOM System Flow Dashboard Launcher")
    parser.add_argument("--mock", action="store_true", help="Run in mock data mode")
    parser.add_argument("--port", type=int, default=5555, help="AXIOM Engine IPC port")
    parser.add_argument("--build", action="store_true", help="Force rebuild before launching")
    
    args = parser.parse_args()

    # Paths
    root_dir = Path(__file__).parent.parent.absolute()
    build_dir = root_dir / "build" / "axui_dashboard"
    
    executable_name = "axiom_dashboard.exe" if sys.platform == "win32" else "axiom_dashboard"
    executable_path = build_dir / "qml" / executable_name

    if args.build or not executable_path.exists():
        print("Building AXIOM Dashboard...")
        os.makedirs(build_dir, exist_ok=True)
        
        cmake_cmd = [
            "cmake",
            "-S", str(root_dir / "axui"),
            "-B", str(build_dir),
            "-DCMAKE_BUILD_TYPE=Release"
        ]
        
        try:
            subprocess.run(cmake_cmd, check=True)
        except subprocess.CalledProcessError:
            print("CMake configuration failed. Attempting a clean build...")
            shutil.rmtree(build_dir, ignore_errors=True)
            os.makedirs(build_dir, exist_ok=True)
            subprocess.run(cmake_cmd, check=True)

        subprocess.run(["cmake", "--build", str(build_dir), "--config", "Release"], check=True)

    # Launch environment
    env = os.environ.copy()
    
    # CRITICAL: Add Qt/MinGW bin to PATH so DLLs are found
    qt_bin = "C:\\msys64\\ucrt64\\bin"
    if qt_bin not in env.get("PATH", ""):
        env["PATH"] = f"{qt_bin};{env.get('PATH', '')}"
    
    # CRITICAL: Set Qt Plugin Path for MSYS2
    env["QT_QPA_PLATFORM_PLUGIN_PATH"] = "C:\\msys64\\ucrt64\\share\\qt6\\plugins\\platforms"

    if args.mock:
        env["AXIOM_MOCK_MODE"] = "1"
    env["AXIOM_IPC_PORT"] = str(args.port)
    
    # QML Imports
    env["QML_IMPORT_PATH"] = str(root_dir / "axui" / "qml" / "resources")

    print(f"Launching AXIOM Dashboard {'(MOCK MODE)' if args.mock else ''}...")
    try:
        # Simplified: no pipes, let output flow to terminal directly
        subprocess.run(
            [str(executable_path)], 
            env=env,
            check=False
        )
    except KeyboardInterrupt:
        print("\nDashboard stopped.")

if __name__ == "__main__":
    main()
