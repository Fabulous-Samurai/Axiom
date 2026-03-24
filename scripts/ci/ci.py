#!/usr/bin/env python3
"""
AXIOM Local CI/CD Pipeline — Multi-Platform Agent-Friendly
"""

import sys
import os
import argparse
import time
import json
from pathlib import Path

# Fix path to allow importing from the current directory
sys.path.insert(0, str(Path(__file__).parent.parent))

try:
    from ci.utils import (
        CIResult, Status,
        print_header, print_stage, save_results,
        load_config, find_project_root
    )
    from ci.platforms import (
        get_available_platforms, run_platform_ci,
        run_all_platforms, detect_current_platform, docker_available
    )
except ImportError as e:
    print(f"Error importing modules: {e}")
    sys.exit(1)

def list_platforms():
    """Mevcut platformları listele"""
    platforms = get_available_platforms()
    print_header("Available CI Platforms")
    for p in platforms:
        icon = "✅" if p.available else "❌"
        runner_tag = f"[{p.runner}]"
        compiler_tag = f"({p.compiler})"
        print(f"  {icon} {p.name:20} {p.display_name:35} {runner_tag:10} {compiler_tag}")
    print()
    current = detect_current_platform()
    has_docker = docker_available()
    print(f"  Current OS: {current}")
    print(f"  Docker: {'✅ Available' if has_docker else '❌ Not available'}")
    print()

def run_pipeline(
    platform: str = "native",
    stages: list[str] = None,
    clean: bool = False,
    json_only: bool = False
) -> CIResult:
    """Ana CI pipeline"""
    config = load_config()
    start_time = time.time()

    if not json_only:
        print_header(f"AXIOM Local CI — Platform: {platform}")

    if platform == "all":
        result = run_all_platforms(config)
    else:
        # Simplification: logic moved to run_platform_ci
        result = run_platform_ci(platform, config)

    result.total_duration_seconds = time.time() - start_time

    if not json_only:
        print_final(result, platform)
    else:
        print(json.dumps(result.to_dict(), indent=2, ensure_ascii=False))

    save_results(result)
    return result

def print_final(result, platform):
    print(f"\n{'─' * 60}")
    status_str = result.overall_status.value if isinstance(result.overall_status, Status) else result.overall_status
    icon = "✅" if status_str == "PASS" else "❌"
    print(f"\n  {icon} {status_str} — Platform: {platform}")
    print(f"  ⏱️  Total: {result.total_duration_seconds:.1f}s")
    for stage in result.stages:
        s = stage["status"]
        icon = {"PASS": "✅", "FAIL": "❌", "SKIP": "⏭️ ", "WARN": "⚠️ "}.get(s, "?")
        print(f"  {icon} {stage['name']:35} {stage.get('message', '')[:50]}")
    print(f"\n  📁 Results: ci-results/latest.json")
    print(f"{'═' * 60}\n")

def main():
    parser = argparse.ArgumentParser(description="AXIOM Local CI — Multi-Platform Pipeline")
    parser.add_argument("--platform", default="native", help="Target platform")
    parser.add_argument("--platforms", action="store_true", help="List platforms")
    parser.add_argument("--stage", help="Specific stage (ignored for now)")
    parser.add_argument("--json", action="store_true", help="JSON output")
    
    args = parser.parse_args()
    if args.platforms:
        list_platforms()
        return

    result = run_pipeline(platform=args.platform, json_only=args.json)
    sys.exit(0 if (result.overall_status.value if isinstance(result.overall_status, Status) else result.overall_status) == "PASS" else 1)

if __name__ == "__main__":
    main()
