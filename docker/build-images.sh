#!/bin/bash
# AXIOM Zenith - Professional Image Builder & Runner

IMAGE_VERSION="3.1.2"
REGISTRY="axiom-ci"

echo "--------------------------------------------------------"
echo "  [AXIOM] DOCKER INFRASTRUCTURE BUILDER v${IMAGE_VERSION}"
echo "--------------------------------------------------------"

# 1. Build Ubuntu Base
echo "[1/2] Building AXIOM Ubuntu (GCC 14 / C++23)..."
docker build -t ${REGISTRY}:ubuntu-${IMAGE_VERSION} -f Dockerfile.ubuntu .
docker tag ${REGISTRY}:ubuntu-${IMAGE_VERSION} ${REGISTRY}:ubuntu-latest

# 2. Build Fedora Base
echo "[2/2] Building AXIOM Fedora (Clang / Fedora 40)..."
docker build -t ${REGISTRY}:fedora-${IMAGE_VERSION} -f Dockerfile.fedora .
docker tag ${REGISTRY}:fedora-${IMAGE_VERSION} ${REGISTRY}:fedora-latest

echo "--------------------------------------------------------"
echo "  [SUCCESS] Images ready for Local CI usage."
echo "  Usage: ./local-run.sh <os> (ubuntu/fedora)"
echo "--------------------------------------------------------"
