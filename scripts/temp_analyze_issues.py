import json
import sys

def analyze():
    try:
        with open('sonar_issues.json', 'r', encoding='utf-8') as f:
            issues = json.load(f)
    except Exception as e:
        print(f"Error loading JSON: {e}")
        return

    print('--- RELIABILITY RELATED ISSUES (Top 50) ---')
    # Filter for bugs or issues that might affect reliability
    rel_types = ['BUG', 'VULNERABILITY']
    reliability = [i for i in issues if i.get('type') in rel_types or i.get('severity') in ['BLOCKER', 'CRITICAL']]
    
    # Sorting by severity
    sev_map = {'BLOCKER': 0, 'CRITICAL': 1, 'MAJOR': 2, 'MINOR': 3, 'INFO': 4}
    reliability.sort(key=lambda x: sev_map.get(x.get('severity', 'INFO'), 5))
    
    for r in reliability[:50]:
        print(f"{r.get('severity')} | {r.get('type')}: {r.get('component').split(':')[-1]}:{r.get('line')} - {r.get('message')}")

if __name__ == "__main__":
    analyze()
