import os
import urllib.request

TLC_URL = "https://github.com/tlaplus/tlaplus/releases/download/v1.8.0/tla2tools.jar"
DEST = "tla2tools.jar"

def setup():
    if not os.path.exists(DEST):
        print(f"Downloading TLC from {TLC_URL}...")
        urllib.request.urlretrieve(TLC_URL, DEST)
        print("Download complete.")
    else:
        print("TLC already exists.")

if __name__ == '__main__':
    setup()
