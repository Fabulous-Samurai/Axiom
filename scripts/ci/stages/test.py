"""AXIOM CI — Native Test Stage"""
import os
import time
from ..utils import StageResult, Status, run_command, find_project_root

def run_tests(config):
    root = find_project_root()
    build_cfg = config.get("build", {})
    build_dir = build_cfg.get("build_dir", "build-ci")

    stage = StageResult("TEST")
    start = time.time()

    test_cmd = f"cd {build_dir} && ctest --output-on-failure -j8"

    rc, stdout, stderr, _ = run_command(test_cmd, cwd=str(root))
    if rc != 0:
        stage.status = Status.FAIL
        stage.message = f"Tests failed with exit code {rc}"
        stage.errors = [stderr]
    else:
        stage.status = Status.PASS
        stage.message = "All native tests passed"

    stage.duration_seconds = time.time() - start
    return stage
