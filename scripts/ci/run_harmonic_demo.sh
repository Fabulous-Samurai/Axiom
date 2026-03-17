#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build_harmonic_demo"
mkdir -p "$BUILD_DIR"
pushd "$BUILD_DIR"
if ! command -v cmake >/dev/null 2>&1; then
  echo "cmake not found; cannot build demo."
  exit 2
fi

cmake -DAXIOM_BUILD_DEMOS=ON -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --target harmonic_arena_demo -j || true

if [ -f "harmonic_arena_demo" ] || [ -f "harmonic_arena_demo.exe" ]; then
  echo "Running demo..."
  if [ -f "harmonic_arena_demo" ]; then
    ./harmonic_arena_demo
  else
    ./harmonic_arena_demo.exe
  fi
else
  echo "Demo binary not found after build."
fi
popd
