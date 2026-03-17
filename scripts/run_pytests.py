import os
import sys
import tempfile
import importlib.util
import xml.etree.ElementTree as ET


def ensure_reports_dir(path: str) -> bool:
    try:
        os.makedirs(path, exist_ok=True)
        # probe write
        probe = os.path.join(path, ".write_test")
        with open(probe, "w", encoding="utf-8") as f:
            f.write("ok")
        os.remove(probe)
        return True
    except Exception:
        return False


def run_pytest(junit_path: str) -> int:
    try:
        import pytest
    except Exception as e:
        print(f"Pytest import failed: {e}")
        return 1

    # Make runs deterministic and headless by default
    os.environ.setdefault("PYTEST_DISABLE_PLUGIN_AUTOLOAD", "1")
    os.environ.setdefault("MPLBACKEND", "Agg")

    args = [
        "-q",
        "-r", "a",
        "-vv",
        "--durations=25",
        "--maxfail=1",
        "--junitxml", junit_path,
        "tests",
    ]

    # Optional quick filter to avoid heavy/GUI/perf tests (default on unless AXIOM_QUICK=0)
    if os.environ.get("AXIOM_QUICK", "1") != "0":
        args.extend(["-k", "not render and not plot and not perf and not comprehensive and not fast_render"])

    # If pytest-timeout is available, add a safety timeout
    if importlib.util.find_spec("pytest_timeout") is not None:
        # Explicitly load the plugin since autoload is disabled
        args.extend(["-p", "pytest_timeout", "--timeout=30", "--timeout-method=thread"])
    else:
        print("pytest-timeout not installed; proceeding without timeouts.")

    print("Running pytest with args:", " ".join(args))
    return pytest.main(args)
def parse_junit(junit_path: str) -> dict:
    if not os.path.exists(junit_path):
        return {"exists": False}

    tree = ET.parse(junit_path)
    root = tree.getroot()

    def attrs(node):
        return {
            "tests": int(node.attrib.get("tests", 0)),
            "failures": int(node.attrib.get("failures", 0)),
            "errors": int(node.attrib.get("errors", 0)),
            "skipped": int(node.attrib.get("skipped", node.attrib.get("disabled", 0))),
            "time": float(node.attrib.get("time", 0.0)),
            "name": node.attrib.get("name", ""),
        }

    summary = {"exists": True, "tests": 0, "failures": 0, "errors": 0, "skipped": 0, "time": 0.0}
    failed_cases = []
    skipped_cases = []

    if root.tag == "testsuite":
        meta = attrs(root)
        for tc in root.iter("testcase"):
            cls = tc.attrib.get("classname", "")
            name = tc.attrib.get("name", "")
            if tc.find("failure") is not None or tc.find("error") is not None:
                failed_cases.append(f"{cls}::{name}")
            if tc.find("skipped") is not None:
                skipped_cases.append(f"{cls}::{name}")
        summary.update(meta)
    elif root.tag == "testsuites":
        for ts in root.findall("testsuite"):
            meta = attrs(ts)
            summary["tests"] += meta["tests"]
            summary["failures"] += meta["failures"]
            summary["errors"] += meta["errors"]
            summary["skipped"] += meta["skipped"]
            summary["time"] += meta["time"]
            for tc in ts.iter("testcase"):
                cls = tc.attrib.get("classname", "")
                name = tc.attrib.get("name", "")
                if tc.find("failure") is not None or tc.find("error") is not None:
                    failed_cases.append(f"{cls}::{name}")
                if tc.find("skipped") is not None:
                    skipped_cases.append(f"{cls}::{name}")
    else:
        # Unknown format
        pass

    summary["failed_cases"] = failed_cases
    summary["skipped_cases"] = skipped_cases
    return summary


def write_summary(summary_path: str, junit_summary: dict, exit_code: int) -> None:
    lines = []
    if not junit_summary.get("exists"):
        lines.append("JUnit report not found. Pytest may not have produced output.")
    else:
        total = junit_summary.get("tests", 0)
        fails = junit_summary.get("failures", 0)
        errs = junit_summary.get("errors", 0)
        skips = junit_summary.get("skipped", 0)
        time_s = junit_summary.get("time", 0.0)
        lines.append(f"Total: {total}  Failures: {fails}  Errors: {errs}  Skipped: {skips}  Time: {time_s:.2f}s")
        if fails or errs:
            lines.append("")
            lines.append("Failed/Errored Testcases:")
            for item in junit_summary.get("failed_cases", []):
                lines.append(f" - {item}")
        if skips:
            lines.append("")
            lines.append("Skipped Testcases:")
            for item in junit_summary.get("skipped_cases", []):
                lines.append(f" - {item}")

    lines.append("")
    lines.append(f"Pytest exit code: {exit_code}")

    try:
        with open(summary_path, "w", encoding="utf-8") as f:
            f.write("\n".join(lines))
    except Exception as e:
        print(f"Failed to write summary to {summary_path}: {e}")


def main() -> int:
    repo_root = os.path.dirname(os.path.dirname(__file__))
    preferred_dir = os.path.join(repo_root, "docs", "reports")
    # Try preferred location first; if blocked, fall back to %TEMP%
    if ensure_reports_dir(preferred_dir):
        reports_dir = preferred_dir
        location = "repo"
    else:
        temp_dir = os.path.join(tempfile.gettempdir(), "axiom_engine_reports")
        ok = ensure_reports_dir(temp_dir)
        reports_dir = temp_dir if ok else preferred_dir  # last resort
        location = "temp" if ok else "repo (no write probe)"

    junit_path = os.path.join(reports_dir, "pytest-results.xml")
    summary_path = os.path.join(reports_dir, "pytest-summary.txt")

    code = run_pytest(junit_path)
    junit_summary = parse_junit(junit_path)
    write_summary(summary_path, junit_summary, code)

    print(f"Report location: {reports_dir} ({location})")
    print(f"JUnit XML: {junit_path}  exists={junit_summary.get('exists', False)}")
    print(f"Summary:  {summary_path}")
    return code


if __name__ == "__main__":
    sys.exit(main())
