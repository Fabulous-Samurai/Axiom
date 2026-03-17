import requests
import json

# Load configuration from project.json
def load_config():
    with open('.vscode/project.json', 'r') as config_file:
        return json.load(config_file)

# Trigger SonarQube analysis
def trigger_sonar_analysis(config):
    url = f"{config['sonarURL']}/api/ce/submit"
    headers = {
        "Authorization": f"Bearer {config['token']}"
    }
    data = {
        "projectKey": config["project"],
        "branch": "master",  # Update this if you want to analyze a different branch
    }
    response = requests.post(url, headers=headers, data=data)

    if response.status_code == 200:
        print("SonarQube analysis triggered successfully.")
        print("Response:", response.json())
    else:
        print("Failed to trigger SonarQube analysis.")
        print("Status Code:", response.status_code)
        print("Response:", response.text)

if __name__ == "__main__":
    try:
        config = load_config()
        trigger_sonar_analysis(config)
    except Exception as e:
        print("Error:", str(e))