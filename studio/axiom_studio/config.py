import json
import os

CONFIG_FILE = "config.json"

def load_config():
    if os.path.exists(CONFIG_FILE):
        with open(CONFIG_FILE, 'r') as f:
            try:
                return json.load(f)
            except:
                pass
    return {}

def save_config(config_data):
    current = load_config()
    current.update(config_data)
    with open(CONFIG_FILE, 'w') as f:
        json.dump(current, f)