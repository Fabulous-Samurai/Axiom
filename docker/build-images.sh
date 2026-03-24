#!/bin/bash
set -e

echo "════════════════════════════════════════════════════════"
echo "  AXIOM — Building CI Docker Images"
echo "════════════════════════════════════════════════════════"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo ""
echo "[1/2] Building Ubuntu 22.04 image..."
docker build \
    -f "${SCRIPT_DIR}/Dockerfile.ubuntu" \
    -t axiom-ci:ubuntu \
    "${SCRIPT_DIR}"

echo ""
echo "[2/2] Building Fedora 39 image..."
docker build \
    -f "${SCRIPT_DIR}/Dockerfile.fedora" \
    -t axiom-ci:fedora \
    "${SCRIPT_DIR}"

echo ""
echo "✅ Images ready:"
docker images | grep axiom-ci
echo ""
