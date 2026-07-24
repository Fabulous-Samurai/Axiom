#!/bin/bash
# AXIOM Zenith - Dockerized Formal Verification Runner

IMAGE="axiom-ci:ubuntu-latest"
TLA_DIR="/workspace/formal/tla"
JAR_URL="https://github.com/tlaplus/tlaplus/releases/download/v1.8.0/tla2tools.jar"

echo "--------------------------------------------------------"
echo "  [AXIOM] FORMAL VERIFICATION ENGINE (DOCKER MODE)      "
echo "--------------------------------------------------------"

# 1. Check if docker image exists
if [[ "$(docker images -q $IMAGE 2> /dev/null)" == "" ]]; then
    echo "[ERROR] Docker image $IMAGE not found. Run build-images.sh first."
    exit 1
fi

# 2. Command selector
MODEL=${1:-"MantisSecureVault.tla"}
WORKERS=$(nproc)

echo "[EXEC] Running TLC Model Checker on: $MODEL ($WORKERS threads)"

# 3. Run in container
# We mount the whole project but focus the execution on the TLA directory
docker run --rm \
    -v "$(pwd)/..:/workspace" \
    -w "$TLA_DIR" \
    $IMAGE \
    bash -c "
        if [ ! -f tla2tools.jar ]; then
            echo '[INFO] Downloading TLA2Tools...';
            curl -L $JAR_URL -o tla2tools.jar;
        fi

        # Run TLC
        java -Xmx4G -cp tla2tools.jar tlc2.TLC \
            -workers $WORKERS \
            -deadlock \
            -gzip \
            $MODEL
    "

echo "--------------------------------------------------------"
echo "  [DONE] Formal check completed. Check logs for details."
echo "--------------------------------------------------------"
