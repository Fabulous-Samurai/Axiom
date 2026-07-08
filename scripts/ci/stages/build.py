"""AXIOM CI — Native Build Stage"""
import os
import time
from ..utils import StageResult, Status, run_command, find_project_root

def run_build(config, clean=False):
    root = find_project_root()
    build_cfg = config.get("build", {})
    build_dir = build_cfg.get("build_dir", "build-ci")

    stage = StageResult("BUILD")
    start = time.time()

    if clean and (root / build_dir).exists():
        import shutil
        shutil.rmtree(root / build_dir)

    # Configuration for local native build
    if os.name == 'nt':
        # Windows specifics
        configure_cmd = f"cmake -B {build_dir} -G Ninja -DCMAKE_BUILD_TYPE=Debug -DAXIOM_ENABLE_TELEMETRY=ON"
    else:
        # Linux/Mac specifics
        configure_cmd = f"cmake -B {build_dir} -G Ninja -DCMAKE_BUILD_TYPE=Debug -DAXIOM_ENABLE_TELEMETRY=ON"

    build_cmd = f"cmake --build {build_dir} -j8"

    rc, stdout, stderr, _ = run_command(configure_cmd, cwd=str(root))
    if rc != 0:
        stage.status = Status.FAIL
        stage.message = "CMake configuration failed"
        stage.errors = [stderr]
        return stage

    rc, stdout, stderr, _ = run_command(build_cmd, cwd=str(root))
    if rc != 0:
        stage.status = Status.FAIL
        stage.message = "Build failed"
        stage.errors = [stderr]
        return stage

    stage.status = Status.PASS
    stage.message = "Build successful"
    stage.duration_seconds = time.time() - start
    return stage
