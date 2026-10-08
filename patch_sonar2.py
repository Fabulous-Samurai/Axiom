import re
with open(".github/workflows/axiom_zenith_ci.yml", "r") as f:
    content = f.read()

# According to typical SonarCloud rules for CI:
# "pip install" should use "--no-cache-dir" or similar, or it wants to avoid executing pip directly.
# Let's search memory. The memory explicitly says: "ensure all Python dependencies installed via `pip` are strictly pinned to a specific version (e.g., `python -m pip install pytest==8.2.2`)."
# BUT maybe I need to pin `python -m pip install pre-commit==4.6.2` to a specific hash? Or maybe I used the wrong version for the other ones?
# Or maybe the rule is about `curl | bash` or `wget`?
# Let's check lines 25, 35, 56, 65, 102
# Line 25: python -m pip install pre-commit==4.6.2
# Line 35: wget https://github.com/tlaplus/tlaplus/releases/download/v1.8.0/tla2tools.jar -O tla2tools.jar
# Line 56: python -m pip install nanobind==3.1.0 pytest==9.1.1 numpy==2.5.3 matplotlib==3.11.2
# Line 65: curl -sSLo $HOME/build-wrapper-linux-x86.zip https://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip
# Line 102: python3 -m pip install pytest==9.1.1 numpy==2.5.3 matplotlib==3.11.2

# Ah, it's warning about downloading files over HTTP(S) and not verifying their checksums/hashes!
# Rule: "Make sure that downloading this file over the network without verifying its checksum is safe."
# This applies to `wget` and `curl` and `pip install` without hashes!
# To fix `pip install`, we can use `--require-hashes` or just use the `hash:` suffix if it's supported, or just ignore it if it's not the pip install.
# Wait! SonarCloud rule python:S6730 or similar is about "Dependency pinning". Maybe `--require-hashes`?
# Let's just try to remove `pip install` and use requirements.txt? No, memory says to strictly pin versions.
# Let's look at the warnings again:
# Line 25, 35, 56, 65, 102
# All of these lines are downloading things from the internet:
# Line 25: pip install pre-commit
# Line 35: wget tla2tools.jar
# Line 56: pip install nanobind pytest...
# Line 65: curl build-wrapper
# Line 102: pip install pytest...
# To fix wget and curl, we can add a sha256sum verification step.
# For pip install, we might need to use `hash` checking or simply pinning is enough and the error is actually because we didn't pin something else?
