#!/usr/bin/env bash
# [AXIOM_DEPLOYMENT_FORGE] v1.0 - Autonomous Silicon Discovery
set -euo pipefail

echo "--------------------------------------------------------"
echo "  [AXIOM] PROJECT AXIOM: PRINCIPAL RELEASE ARCHITECT    "
echo "  Targeting Zero-Latency / Deterministic Zenith Core    "
echo "--------------------------------------------------------"

# 1. Silicon Discovery
ARCH_FLAG="native"
CPU_FEATURES=""

if [[ "$(uname)" == "Linux" ]]; then
    CPU_FEATURES=$(lscpu | grep Flags || true)
elif [[ "$(uname)" == "Darwin" ]]; then
    CPU_FEATURES=$(sysctl -a | grep machdep.cpu.features || true)
fi

echo "[AXIOM] Probing Silicon Architecture..."

if [[ $CPU_FEATURES == *"avx512"* ]]; then
    echo "[FOUND] AVX-512 Support Detected. Unleashing Giga-Vector Units."
    ARCH_FLAG="skylake-avx512"
elif [[ $CPU_FEATURES == *"avx2"* ]]; then
    echo "[FOUND] AVX2 Support Detected. Optimizing for Haswell+ Pipeline."
    ARCH_FLAG="haswell"
elif [[ $CPU_FEATURES == *"neon"* || "$(uname -m)" == "arm64" ]]; then
    echo "[FOUND] ARM NEON Detected. Configuring Apple/Cortex-A Engine."
    ARCH_FLAG="native"
else
    echo "[WARN] Generic Architecture or Legacy Silicon. Efficiency limited."
    ARCH_FLAG="native"
fi

# 2. Build Environment Setup
command -v cmake >/dev/null 2>&1 || { echo "[ERROR] cmake not found"; exit 1; }
command -v ninja >/dev/null 2>&1 || { echo "[ERROR] ninja not found"; exit 1; }

echo "[AXIOM] Initializing Build Forge for architecture: $ARCH_FLAG"

# 3. Compilation with Native Optimization
# We bypass generic binaries to ensure L1 cache locality and Vector Port saturation.
mkdir -p build && cd build

# Allow dynamic Build Type for CI/CD flexibility
BUILD_TYPE=${BUILD_TYPE:-Release}
OPT_FLAG="-O3"
if [ "$BUILD_TYPE" == "Debug" ]; then
    OPT_FLAG="-Og -g"
fi

# Respect environment CXXFLAGS and LDFLAGS if present
EXTRA_CXX_FLAGS=${CXXFLAGS:-""}
EXTRA_LD_FLAGS=${LDFLAGS:-""}

cmake -G Ninja .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_CXX_FLAGS="$EXTRA_CXX_FLAGS -march=$ARCH_FLAG -mtune=$ARCH_FLAG $OPT_FLAG -flto" \
    -DCMAKE_EXE_LINKER_FLAGS="$EXTRA_LD_FLAGS" \
    -DCMAKE_SHARED_LINKER_FLAGS="$EXTRA_LD_FLAGS" \
    -DAXIOM_ENABLE_TELEMETRY=ON

echo "[AXIOM] Building Zenith Core and Tests ($BUILD_TYPE)..."
ninja

# 4. Final Validation
if [[ -x ./axiom ]]; then
    echo "--------------------------------------------------------"
    echo "[SUCCESS] AXIOM ZENITH DEPLOYED SUCCESSFULLY"
    echo "Architecture: $(uname -m) / $ARCH_FLAG"
    echo "--------------------------------------------------------"
else
    echo "[ERROR] Zenith Build Failed. Check compiler logs."
    exit 1
fi
