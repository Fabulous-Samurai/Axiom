import requests
import json

def get_latest_runs(owner, repo):
    url = f"https://api.github.com/repos/{owner}/{repo}/actions/runs"
    response = requests.get(url)
    if response.status_code == 200:
        return response.json()['workflow_runs']
    else:
        print(f"Failed to fetch runs: {response.status_code}")
        return []

def get_run_jobs(owner, repo, run_id):
    url = f"https://api.github.com/repos/{owner}/{repo}/actions/runs/{run_id}/jobs"
    response = requests.get(url)
    if response.status_code == 200:
        return response.json()['jobs']
    else:
        print(f"Failed to fetch jobs: {response.status_code}")
        return []

owner = "Fabulous-Samurai"
repo = "Axiom"

runs = get_latest_runs(owner, repo)
if runs:
    latest_run = runs[0]
    print(f"Latest Run: {latest_run['id']} - {latest_run['display_title']} ({latest_run['status']} / {latest_run['conclusion']})")
    
    jobs = get_run_jobs(owner, repo, latest_run['id'])
    for job in jobs:
        print(f"Job: {job['name']} - {job['status']} / {job['conclusion']}")
        if job['conclusion'] == 'failure':
            print(f"  URL: {job['html_url']}")
            # We can't easily get the logs without a token, but we see which ones failed.
else:
    print("No runs found.")
