import os
import subprocess
import time
import json
from enum import Enum
from pathlib import Path

class Status(Enum):
    PASS = "PASS"
    FAIL = "FAIL"
    SKIP = "SKIP"
    WARN = "WARN"

class StageResult:
    def __init__(self, name, status=Status.SKIP, message=""):
        self.name = name
        self.status = status
        self.message = message
        self.duration_seconds = 0.0
        self.details = {}
        self.errors = []

    def to_dict(self):
        return {
            "name": self.name,
            "status": self.status.value if isinstance(self.status, Status) else self.status,
            "message": self.message,
            "duration_seconds": self.duration_seconds,
            "details": self.details,
            "errors": self.errors
        }

class CIResult:
    def __init__(self):
        self.stages = []
        self.overall_status = Status.PASS
        self.total_duration_seconds = 0.0

    def add_stage(self, stage):
        if isinstance(stage, StageResult):
            self.stages.append(stage.to_dict())
            if stage.status == Status.FAIL:
                self.overall_status = Status.FAIL
        else:
            self.stages.append(stage)
            if stage.get("status") == "FAIL":
                self.overall_status = Status.FAIL

    def to_dict(self):
        return {
            "overall_status": self.overall_status.value if isinstance(self.overall_status, Status) else self.overall_status,
            "total_duration_seconds": self.total_duration_seconds,
            "stages": self.stages
        }

def run_command(cmd, cwd=None, timeout=None, capture=True):
    start = time.time()
    try:
        process = subprocess.run(
            cmd,
            cwd=cwd,
            capture_output=capture,
            text=True,
            timeout=timeout,
            shell=isinstance(cmd, str)
        )
        duration = time.time() - start
        return process.returncode, process.stdout or "", process.stderr or "", duration
    except subprocess.TimeoutExpired:
        return -1, "", "Timeout", time.time() - start
    except Exception as e:
        return -1, "", str(e), time.time() - start

def find_project_root():
    curr = Path(__file__).resolve().parent
    while curr.parent != curr:
        if (curr / ".git").exists() or (curr / "CMakeLists.txt").exists():
            return curr
        curr = curr.parent
    return Path.cwd()

def load_config():
    root = find_project_root()
    cfg_path = root / "scripts" / "ci" / "ci-config.json"
    if cfg_path.exists():
        with open(cfg_path, 'r') as f:
            return json.load(f)
    return {"build": {"build_dir": "build-ci"}}

def print_header(text):
    print(f"\n{'═' * 60}")
    print(f"  {text}")
    print(f"{'═' * 60}\n")

def print_stage(name, status, duration, message):
    icon = {"PASS": "✅", "FAIL": "❌", "SKIP": "⏭️ ", "WARN": "⚠️ "}.get(status.value if isinstance(status, Status) else status, "?")
    print(f"  {icon} {name:20} {duration:5.1f}s  {message}")

def save_results(result):
    root = find_project_root()
    out_dir = root / "ci-results"
    out_dir.mkdir(exist_ok=True)
    with open(out_dir / "latest.json", "w") as f:
        json.dump(result.to_dict(), f, indent=2)

def generate_text_report(result):
    # Simplified text report
    pass
