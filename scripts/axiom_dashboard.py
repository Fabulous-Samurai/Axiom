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
    build_dir = root_dir / "out" / "default-ninja"

    executable_name = "axiom_dashboard.exe" if sys.platform == "win32" else "axiom_dashboard"
    # The dashboard is located in products/dashboard within the build tree
    executable_path = build_dir / "products" / "dashboard" / executable_name

    if args.build or not executable_path.exists():
        print("AXIOM Dashboard binary not found in out/default-ninja.")
        print("Please run 'cmake --build out/default-ninja' first or use the main build system.")
        # Optional: could trigger main build here, but let's keep it separate for control
        # sys.exit(1) if you want to force manual build


    # Launch environment
    env = os.environ.copy()

    # CRITICAL: Add Qt/MinGW bin to PATH so DLLs are found
    qt_bin = "C:\\msys64\\ucrt64\\bin"
    qt_lib_bin = "C:\\msys64\\ucrt64\\lib\\qt6\\bin"
    if qt_bin not in env.get("PATH", ""):
        env["PATH"] = f"{qt_bin};{qt_lib_bin};{env.get('PATH', '')}"

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
