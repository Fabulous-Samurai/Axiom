import json
import sys
import os

def check_perf(current_file, baseline_file, threshold=0.05):
    if not os.path.exists(current_file):
        print(f"Error: Current benchmark file {current_file} not found.")
        return 1

    if not os.path.exists(baseline_file):
        print(f"No baseline found at {baseline_file}. Saving current as baseline.")
        with open(current_file, 'r') as f:
            data = json.load(f)
        with open(baseline_file, 'w') as f:
            json.dump(data, f, indent=2)
        return 0

    with open(current_file, 'r') as f:
        current_data = json.load(f)
    with open(baseline_file, 'r') as f:
        baseline_data = json.load(f)

    regressions = []

    # Google Benchmark JSON format: { "benchmarks": [ { "name": "...", "cpu_time": ... }, ... ] }
    current_benchs = { b['name']: b['cpu_time'] for b in current_data.get('benchmarks', []) }
    baseline_benchs = { b['name']: b['cpu_time'] for b in baseline_data.get('benchmarks', []) }

    for name, curr_time in current_benchs.items():
        if name in baseline_benchs:
            base_time = baseline_benchs[name]
            diff = (curr_time - base_time) / base_time
            if diff > threshold:
                regressions.append(f"{name}: {base_time:.2f} -> {curr_time:.2f} (+{diff*100:.2f}%)")

    if regressions:
        print("❌ PERFORMANCE REGRESSION DETECTED!")
        for r in regressions:
            print(f"  - {r}")
        return 1

    print("✅ Performance within acceptable thresholds.")
    return 0

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python check_perf_regression.py <current_json> <baseline_json> [threshold]")
        sys.exit(1)

    curr = sys.argv[1]
    base = sys.argv[2]
    thresh = float(sys.argv[3]) if len(sys.argv) > 3 else 0.05
    sys.exit(check_perf(curr, base, thresh))
