#!/bin/bash
echo "🚀 AXIOM ENGINE v3.0 - CENTRALIZED BUILD SYSTEM"
echo "========================================"

# Check if CMakePresets.json exists
if [ ! -f "CMakePresets.json" ]; then
    echo "❌ Error: CMakePresets.json not found!"
    exit 1
fi

echo "🏗️ Configuring with preset: default-ninja..."
cmake --preset default-ninja

echo "🏎️ Building AXIOM Engine..."
cmake --build out/default-ninja -j $(nproc)

echo "✅ AXIOM Engine v3.0 build complete!"
echo "📍 Executable: out/default-ninja/axiom"
echo "🧪 Test suite: out/default-ninja/run_tests"
