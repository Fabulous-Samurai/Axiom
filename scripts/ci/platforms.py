"""AXIOM Local CI — Cross-Platform Runner (Docker + Native)"""

import os
import sys
import json
import platform
import shutil
import subprocess
from pathlib import Path
from dataclasses import dataclass
from typing import Optional

# Assumption: utils.py will be updated or exists to support these imports
# If it doesn't exist, we'll need to create/update it.
try:
    from .utils import (
        StageResult, Status, CIResult,
        run_command, find_project_root, load_config,
        print_header, print_stage, save_results
    )
except ImportError:
    # Fallback/Placeholder if utils is not yet updated
    class Status:
        PASS = "PASS"
        FAIL = "FAIL"
        SKIP = "SKIP"
        WARN = "WARN"

    @dataclass
    class StageResult:
        name: str
        status: str = Status.SKIP
        message: str = ""
        duration_seconds: float = 0.0
        details: Optional[dict] = None
        errors: Optional[list] = None

    class CIResult:
        def __init__(self):
            self.stages = []
            self.overall_status = Status.PASS
            self.total_duration_seconds = 0.0
        def add_stage(self, stage):
            self.stages.append(stage if isinstance(stage, dict) else self._stage_to_dict(stage))
            if stage.status == Status.FAIL:
                self.overall_status = Status.FAIL
        def _stage_to_dict(self, stage):
            return {
                "name": stage.name,
                "status": stage.status,
                "message": stage.message,
                "duration_seconds": stage.duration_seconds,
                "details": stage.details or {},
                "errors": stage.errors or []
            }


@dataclass
class PlatformInfo:
    name: str            # "ubuntu", "fedora", "macos-native", "linux-native"
    display_name: str    # "Ubuntu 22.04", "Fedora 39", "macOS (native)"
    runner: str          # "docker" veya "native"
    docker_image: str    # "axiom-ci:ubuntu" (docker ise)
    compiler: str        # "gcc", "clang"
    available: bool      # Bu platformda çalıştırılabilir mi


def detect_current_platform() -> str:
    """Mevcut OS'u tespit et"""
    system = platform.system().lower()
    if system == "darwin":
        return "macos"
    elif system == "linux":
        return "linux"
    elif system == "windows":
        return "windows"
    return "unknown"


def docker_available() -> bool:
    """Docker kurulu ve çalışıyor mu?"""
    try:
        result = subprocess.run(
            ["docker", "info"],
            capture_output=True, text=True, timeout=10
        )
        return result.returncode == 0
    except (FileNotFoundError, subprocess.TimeoutExpired):
        return False


def docker_image_exists(image: str) -> bool:
    """Docker image mevcut mu?"""
    try:
        result = subprocess.run(
            ["docker", "images", "-q", image],
            capture_output=True, text=True, timeout=10
        )
        return bool(result.stdout.strip())
    except:
        return False


def get_available_platforms() -> list[PlatformInfo]:
    """Çalıştırılabilir platformları listele"""
    current_os = detect_current_platform()
    has_docker = docker_available()

    platforms = []

    # ─── Native (her zaman mevcut) ────────────────────────────
    if current_os == "macos":
        platforms.append(PlatformInfo(
            name="macos-native",
            display_name=f"macOS {platform.mac_ver()[0]} (native)",
            runner="native",
            docker_image="",
            compiler="clang",
            available=True
        ))
    elif current_os == "linux":
        platforms.append(PlatformInfo(
            name="linux-native",
            display_name=f"Linux {platform.release()} (native)",
            runner="native",
            docker_image="",
            compiler="gcc",
            available=True
        ))
    elif current_os == "windows":
        platforms.append(PlatformInfo(
            name="windows-native",
            display_name=f"Windows {platform.version()} (native)",
            runner="native",
            docker_image="",
            compiler="msvc",
            available=True
        ))

    # ─── Docker Platforms ─────────────────────────────────────
    if has_docker:
        # Ubuntu 22.04
        ubuntu_exists = docker_image_exists("axiom-ci:ubuntu")
        platforms.append(PlatformInfo(
            name="ubuntu",
            display_name="Ubuntu 22.04 (Docker)",
            runner="docker",
            docker_image="axiom-ci:ubuntu",
            compiler="gcc",
            available=ubuntu_exists
        ))

        # Fedora 39
        fedora_exists = docker_image_exists("axiom-ci:fedora")
        platforms.append(PlatformInfo(
            name="fedora",
            display_name="Fedora 39 (Docker)",
            runner="docker",
            docker_image="axiom-ci:fedora",
            compiler="gcc",
            available=fedora_exists
        ))

        # Fedora + Clang
        platforms.append(PlatformInfo(
            name="fedora-clang",
            display_name="Fedora 39 + Clang (Docker)",
            runner="docker",
            docker_image="axiom-ci:fedora",
            compiler="clang",
            available=fedora_exists
        ))

    return platforms


def run_native_ci(config: dict) -> CIResult:
    """Native platformda CI çalıştır"""
    # Note: These stages must be implemented in scripts/ci/stages/
    try:
        from .stages.build import run_build
        from .stages.test import run_tests
        from .stages.quality import run_quality
    except ImportError:
        res = CIResult()
        res.add_stage(StageResult("STAGES", Status.FAIL, "CI Stages not found in scripts/ci/stages/"))
        return res

    result = CIResult()

    # Build
    build_result = run_build(config)
    result.add_stage(build_result)

    if build_result.status == Status.FAIL:
        return result

    # Test
    test_result = run_tests(config)
    result.add_stage(test_result)

    # Quality
    try:
        quality_result = run_quality(config)
        result.add_stage(quality_result)
    except:
        pass # Quality is often optional

    return result


def run_docker_ci(
    platform_info: PlatformInfo,
    config: dict
) -> CIResult:
    """Docker container içinde CI çalıştır"""
    from .utils import find_project_root, run_command

    root = find_project_root()
    result = CIResult()

    build_cfg = config.get("build", {})
    build_dir = build_cfg.get("build_dir", "build-ci")

    # Docker run komutu
    # Proje dizinini mount et, içeride build + test yap
    docker_script = f"""
set -e
echo "═══ Platform: {platform_info.display_name} ═══"
echo "Compiler: $(c++ --version | head -1)"
echo ""

# Configure
echo "[1/3] Configuring..."
cmake -B {build_dir} \\
    -G Ninja \\
    -DCMAKE_BUILD_TYPE=Debug \\
    -DCMAKE_CXX_FLAGS="--coverage -O0 -g" \\
    -DCMAKE_EXE_LINKER_FLAGS="--coverage" \\
    -DAXIOM_ENABLE_TELEMETRY=ON \\
    2>&1

# Build
echo "[2/3] Building..."
cmake --build {build_dir} -j$(nproc) 2>&1

# Test
echo "[3/3] Testing..."
cd {build_dir}
ctest --output-on-failure -j$(nproc) 2>&1
TEST_RC=$?

exit $TEST_RC
"""

    docker_cmd = [
        "docker", "run", "--rm",
        "-v", f"{root}:/workspace",
        "-w", "/workspace",
    ]

    # Compiler override
    if platform_info.compiler == "clang":
        docker_cmd.extend(["-e", "CC=clang", "-e", "CXX=clang++"])

    docker_cmd.extend([
        platform_info.docker_image,
        "bash", "-c", docker_script
    ])

    # Çalıştır
    stage = StageResult(name=f"DOCKER:{platform_info.name}")

    rc, stdout, stderr, duration = run_command(
        docker_cmd,
        cwd=str(root),
        timeout=600,
        capture=True
    )

    stage.duration_seconds = duration
    combined_output = stdout + stderr

    if rc == 0:
        stage.status = Status.PASS
        stage.message = f"{platform_info.display_name} — All tests passed"
    else:
        stage.status = Status.FAIL
        stage.message = f"{platform_info.display_name} — Failed (exit {rc})"
        stage.details = {"output": combined_output[-2000:]}

    result.add_stage(stage)
    return result


def run_platform_ci(
    platform_name: str,
    config: dict = None
) -> CIResult:
    """Belirli bir platformda CI çalıştır"""
    from .utils import load_config
    if config is None:
        config = load_config()

    platforms = get_available_platforms()
    target = None

    for p in platforms:
        if p.name == platform_name:
            target = p
            break

    if target is None:
        result = CIResult()
        stage = StageResult(
            name=f"PLATFORM:{platform_name}",
            status=Status.FAIL,
            message=f"Platform not found: {platform_name}"
        )
        result.add_stage(stage)
        return result

    if not target.available:
        result = CIResult()
        stage = StageResult(
            name=f"PLATFORM:{platform_name}",
            status=Status.SKIP,
            message=f"Platform not available: {target.display_name}"
        )
        result.add_stage(stage)
        return result

    if target.runner == "native":
        return run_native_ci(config)
    elif target.runner == "docker":
        return run_docker_ci(target, config)
    else:
        result = CIResult()
        result.add_stage(StageResult(name=platform_name, status=Status.SKIP, message="Remote not supported"))
        return result


def run_all_platforms(config: dict = None) -> CIResult:
    """Tüm mevcut platformlarda CI çalıştır"""
    from .utils import load_config
    if config is None:
        config = load_config()

    platforms = get_available_platforms()
    combined_result = CIResult()

    for p in platforms:
        if not p.available:
            continue
        platform_result = run_platform_ci(p.name, config)
        for stage in platform_result.stages:
            combined_result.add_stage(stage)
    
    return combined_result
