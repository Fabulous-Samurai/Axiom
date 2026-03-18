import requests
import json
import argparse
import os
import sys

class SonarIssuePuller:
    def __init__(self, token, project_key):
        self.token = token
        self.project_key = project_key
        self.base_url = "https://sonarcloud.io/api/issues/search"

    def fetch_issues(self, severities=None, types=None, statuses="OPEN,REOPENED,CONFIRMED"):
        params = {
            "componentKeys": self.project_key,
            "resolved": "false",
            "statuses": statuses
        }
        if severities:
            params["severities"] = severities
        if types:
            params["types"] = types
        else:
            params["types"] = "BUG,VULNERABILITY,CODE_SMELL"

        issues = []
        page = 1
        while True:
            params["p"] = page
            response = requests.get(self.base_url, params=params, auth=(self.token, ""))
            if response.status_code != 200:
                print(f"Error fetching issues: {response.status_code} - {response.text}")
                break
            
            data = response.json()
            current_issues = data.get("issues", [])
            issues.extend(current_issues)
            
            paging = data.get("paging", {})
            if page * paging.get("pageSize", 100) >= paging.get("total", 0):
                break
            page += 1
            
        # Fetch Security Hotspots
        hotspot_url = "https://sonarcloud.io/api/hotspots/search"
        hotspot_params = {
            "projectKey": self.project_key,
            "status": "TO_REVIEW"
        }
        hs_response = requests.get(hotspot_url, params=hotspot_params, auth=(self.token, ""))
        if hs_response.status_code == 200:
            hotspots = hs_response.json().get("hotspots", [])
            for hs in hotspots:
                # Normalize hotspot to look like an issue for the task list
                hs["type"] = "SECURITY_HOTSPOT"
                hs["severity"] = hs.get("vulnerabilityProbability", "MEDIUM")
                issues.append(hs)
            print(f"Fetched {len(hotspots)} security hotspots.")

        return issues

    def export_markdown(self, issues, filename="sonar_tasks.md"):
        with open(filename, "w", encoding="utf-8") as f:
            f.write("# SonarCloud Issues Task List\n\n")
            if not issues:
                f.write("No open issues found! 🎉\n")
                return

            f.write("| Severity | Type | Message | File | Line | Link |\n")
            f.write("|----------|------|---------|------|------|------|\n")
            for issue in issues:
                severity = issue.get("severity")
                type_ = issue.get("type")
                message = issue.get("message")
                component = issue.get("component", "").split(":")[-1]
                line = issue.get("line", "N/A")
                key = issue.get("key")
                link = f"[View](https://sonarcloud.io/project/issues?id={self.project_key}&open={key})"
                f.write(f"| {severity} | {type_} | {message} | `{component}` | {line} | {link} |\n")

    def export_json(self, issues, filename="sonar_issues.json"):
        with open(filename, "w", encoding="utf-8") as f:
            json.dump(issues, f, indent=4)

def main():
    parser = argparse.ArgumentParser(description="Pull issues from SonarCloud")
    parser.add_argument("--token", help="SonarCloud API Token", default=os.getenv("SONAR_TOKEN"))
    parser.add_argument("--project", help="SonarCloud Project Key", default="Fabulous-Samurai_axiom_engine")
    parser.add_argument("--severities", help="Comma-separated severities (CRITICAL,BLOCKER,MAJOR)")
    parser.add_argument("--types", help="Comma-separated types (BUG,VULNERABILITY,CODE_SMELL)")
    parser.add_argument("--format", choices=["markdown", "json", "both"], default="both")

    args = parser.parse_args()

    if not args.token:
        print("Error: SONAR_TOKEN not found. Provide via --token or environment variable.")
        sys.exit(1)

    puller = SonarIssuePuller(args.token, args.project)
    print(f"Fetching issues for {args.project}...")
    issues = puller.fetch_issues(args.severities, args.types)
    print(f"Found {len(issues)} issues.")

    if args.format in ["markdown", "both"]:
        puller.export_markdown(issues)
        print("Exported sonar_tasks.md")
    if args.format in ["json", "both"]:
        puller.export_json(issues)
        print("Exported sonar_issues.json")

if __name__ == "__main__":
    main()
